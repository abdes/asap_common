//        Copyright The Authors 2018.
//    Distributed under the 3-Clause BSD License.
//    (See accompanying file LICENSE or copy at
//   https://opensource.org/licenses/BSD-3-Clause)

#include <common/assert.h>
#include <common/config.h>
#include <common/platform.h>
#include <hedley/hedley.h>

#if ASAP_USE_ASSERTS

#include <array>
#include <cinttypes>  // for PRId64 et.al.
#include <csignal>
#include <cstdarg>  // for va_start, va_end
#include <cstdio>   // for snprintf
#include <cstdlib>
#include <cstring>  // for strncat
#include <string>   // for strstr, strchr

// __has_include is currently supported by GCC and Clang. However GCC 4.9 may
// have issues and returns 1 for 'defined( __has_include )', while
// '__has_include' is actually not supported:
// https://gcc.gnu.org/bugzilla/show_bug.cgi?id=63662
#if defined(__has_include) && (!HEDLEY_GNUC_VERSION || (__GNUC__ + 0) >= 5)
#if __has_include(<cxxabi.h>)
#define ASAP_HAS_CXXABI_H
#endif
#elif defined(__GLIBCXX__) || defined(__GLIBCPP__)
#define ASAP_HAS_CXXABI_H
#endif

#if defined(ASAP_HAS_CXXABI_H)
#include <cxxabi.h>
// For some architectures (mips, mips64, x86, x86_64) cxxabi.h in Android NDK is
// implemented by gabi++ library
// (https://android.googlesource.com/platform/ndk/+/master/sources/cxx-stl/gabi++/),
// which does not implement abi::__cxa_demangle(). We detect this implementation
// by checking the include guard here.
#if defined(__GABIXX_CXXABI_H__)
#undef ASAP_HAS_CXXABI_H
#else
#include <cstddef>
#include <cstdlib>
#endif

namespace {
inline auto demangle_alloc(char const* name) noexcept -> char const*;
inline void demangle_free(char const* name) noexcept;

class scoped_demangled_name {
 private:
  char const* m_p;

 public:
  explicit scoped_demangled_name(char const* name) noexcept
      : m_p(demangle_alloc(name)) {}

  ~scoped_demangled_name() noexcept { demangle_free(m_p); }

  auto get() const noexcept -> char const* { return m_p; }

  scoped_demangled_name(scoped_demangled_name const&) = delete;
  auto operator=(scoped_demangled_name const&)
      -> scoped_demangled_name& = delete;
};

inline auto demangle_alloc(char const* name) noexcept -> char const* {
  int status = 0;
  std::size_t size = 0;
  return abi::__cxa_demangle(name, nullptr, &size, &status);
}

inline void demangle_free(char const* name) noexcept {
  std::free(const_cast<char*>(name));
}

inline auto demangle(char const* name) -> std::string {
  scoped_demangled_name demangled_name(name);
  char const* p = demangled_name.get();
  if (p == nullptr) {
    p = name;
  }
  return p;
}
}  // namespace
#else   // !ASAP_HAS_CXXABI_H
namespace {
inline auto demangle_alloc(char const* name) noexcept -> char const* {
  return name;
}

inline void demangle_free(char const* /*unused*/) noexcept {}

inline auto demangle(char const* name) -> std::string { return name; }
}  // namespace
#endif  // ASAP_HAS_CXXABI_H

#if ASAP_USE_EXECINFO
#include <execinfo.h>

namespace {
void print_backtrace(char* out, int len, int max_depth, void* /*unused*/) {
  void* stack[50];
  int size = ::backtrace(stack, 50);
  char** symbols = ::backtrace_symbols(stack, size);

  for (int i = 1; i < size && len > 0; ++i) {
    int ret = std::snprintf(out, std::size_t(len), "%d: %s\n", i,
                            demangle(symbols[i]).c_str());
    out += ret;
    len -= ret;
    if (i - 1 == max_depth && max_depth > 0) {
      break;
    }
  }

  ::free(symbols);
}
}  // namespace

#elif defined(ASAP_WINDOWS)

#include <mutex>
   // clang-format off
#include "windows.h"
#include "winbase.h"
#include "dbghelp.h"
// clang-form on

namespace {

void print_backtrace(char* out, int len, int max_depth, void* ctx) {
  // all calls to DbgHlp.dll are thread-unsafe. i.e. they all need to be
  // synchronized and not called concurrently. This mutex serializes access
  static std::mutex dbghlp_mutex;
  std::lock_guard<std::mutex> l(dbghlp_mutex);

  CONTEXT context_record;
  if (ctx) {
    context_record = *static_cast<CONTEXT*>(ctx);
  } else {
    // use the current thread's context
    RtlCaptureContext(&context_record);
  }

  int size = 0;
  std::array<void*, 50> stack;

  STACKFRAME64 stack_frame = {};
#if defined(_WIN64)
  int const machine_type = IMAGE_FILE_MACHINE_AMD64;
  stack_frame.AddrPC.Offset = context_record.Rip;
  stack_frame.AddrFrame.Offset = context_record.Rbp;
  stack_frame.AddrStack.Offset = context_record.Rsp;
#else
  int const machine_type = IMAGE_FILE_MACHINE_I386;
  stack_frame.AddrPC.Offset = context_record.Eip;
  stack_frame.AddrFrame.Offset = context_record.Ebp;
  stack_frame.AddrStack.Offset = context_record.Esp;
#endif
  stack_frame.AddrPC.Mode = AddrModeFlat;
  stack_frame.AddrFrame.Mode = AddrModeFlat;
  stack_frame.AddrStack.Mode = AddrModeFlat;
  while (StackWalk64(machine_type, GetCurrentProcess(), GetCurrentThread(),
                     &stack_frame, &context_record, nullptr,
                     &SymFunctionTableAccess64, &SymGetModuleBase64, nullptr) &&
         size < int(stack.size())) {
    stack[size++] = reinterpret_cast<void*>(stack_frame.AddrPC.Offset);
  }

  struct symbol_bundle : SYMBOL_INFO {
    wchar_t name[MAX_SYM_NAME];
  };

  HANDLE p = GetCurrentProcess();
  static bool sym_initialized = false;
  if (!sym_initialized) {
    sym_initialized = true;
    SymInitialize(p, nullptr, true);
  }
  SymRefreshModuleList(p);
  for (int i = 0; i < size && len > 0; ++i) {
    DWORD_PTR frame_ptr = reinterpret_cast<DWORD_PTR>(stack[i]);

    DWORD64 displacement = 0;
    symbol_bundle symbol;
    symbol.MaxNameLen = MAX_SYM_NAME;
    symbol.SizeOfStruct = sizeof(SYMBOL_INFO);
    BOOL const has_symbol = SymFromAddr(p, frame_ptr, &displacement, &symbol);

    DWORD line_displacement = 0;
    IMAGEHLP_LINE64 line = {};
    line.SizeOfStruct = sizeof(IMAGEHLP_LINE64);
    BOOL const has_line = SymGetLineFromAddr64(GetCurrentProcess(), frame_ptr,
                                               &line_displacement, &line);

    int ret = std::snprintf(out, len, "%2d: %p", i, stack[i]);
    out += ret;
    len -= ret;
    if (len <= 0) break;

    if (has_symbol) {
      ret = std::snprintf(out, len, " %s +%-4" PRId64,
                          demangle(symbol.Name).c_str(), displacement);
      out += ret;
      len -= ret;
      if (len <= 0) break;
    }

    if (has_line) {
      ret = std::snprintf(out, len, " %s:%d", line.FileName,
                          int(line.LineNumber));
      out += ret;
      len -= ret;
      if (len <= 0) break;
    }

    ret = std::snprintf(out, len, "\n");
    out += ret;
    len -= ret;

    if (i == max_depth && max_depth > 0) break;
  }
}
}  // namespace

#else  // EXEC_INFO

namespace {
void print_backtrace(char* out, int len, int /*max_depth*/, void* /* ctx */) {
  out[0] = 0;
  std::strncat(out, "<not supported>", std::size_t(len));
}
}  // namespace

#endif  // EXEC_INFO


// GCC and clang will complain if you call printf with a non-literal (and if
// you call it with a literal format string, they will check the format string
// against the provided arguments). The __attribute__((__format__ (__printf__,
// ...) tells the compiler that one of your parameters is a printf format
// string and causes the checking to be applied when that function is called.
// 
// Since the compiler knows that the format string parameter will be checked
// when your function is called, it won't complain about you using that
// parameter as a format string inside your function.
//
// see: https://clang.llvm.org/docs/AttributeReference.html#format
#if HEDLEY_HAS_ATTRIBUTE(format)
#define ASAP_FORMAT(fmt, ellipsis) \
  __attribute__((__format__(__printf__, fmt, ellipsis)))
#else
#define ASAP_FORMAT(fmt, ellipsis)
#endif

namespace {
ASAP_FORMAT(1, 2)
void assert_print(char const* fmt, ...) {
  FILE* out = stderr;
  va_list va;
  va_start(va, fmt);
  std::vfprintf(out, fmt, va);
  va_end(va);
}
}  // namespace

// we deliberately don't want asserts to be marked as no-return, since that
// would trigger warnings in debug builds of any code coming after the assert
#if HEDLEY_HAS_WARNING("-Wmissing-noreturn")
HEDLEY_DIAGNOSTIC_PUSH 
#pragma clang diagnostic ignored "-Wmissing-noreturn"
#endif

namespace asap {
void assert_fail(char const* expr, int line, char const* file,
                 char const* function, char const* value, int kind) {
  char stack[8192];
  stack[0] = '\0';
  print_backtrace(stack, sizeof(stack), 0, nullptr);

  char const* message = "Assertion failed.\n";

  if (kind == 1) {
    message =
        "A precondition of a function has been violated.\n"
        "This indicates a bug in the client application.\n";
  }

  assert_print(
      "%s\n"
      "file: '%s'\n"
      "line: %d\n"
      "function: %s\n"
      "expression: %s\n"
      "%s%s\n"
      "stack:\n"
      "%s\n",
      message, file, line, function, expr, value != nullptr ? value : "",
      value != nullptr ? "\n" : "", stack);
  ::abort();
}
}  // namespace asap

#if HEDLEY_HAS_WARNING("-Wmissing-noreturn")
HEDLEY_DIAGNOSTIC_POP
#endif

#elif !ASAP_USE_ASSERTS

// these are just here to make it possible for a client that built with debug
// enable to be able to link against a release build (just possible, not
// necessarily supported)
namespace {
ASAP_FORMAT(1, 2)
void assert_print(char const*, ...) {}
}  // namespace
namespace asap {
void assert_fail(char const*, int, char const*, char const*, char const*, int) {
}
}  // namespace asap

#endif  // ASAP_USE_ASSERTS
