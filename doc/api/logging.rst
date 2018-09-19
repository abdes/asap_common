Logging
=======

Logging features provided by the ``asap`` ``common`` module are abstracted into asimple and minimalistic interface. Under the hood, the logging framework used
for the implementation is the excellent `spdlog <https://github.com/gabime/spdlog>`_ selected for its performance, light weight
and clean interface.

Design Rationale
----------------

While spdlog or other frameworks offer a nice interface and can achieve the
expected results with no abstraction layer above, they are designed as a
generic frameowkr that caters for many more scenarios than what is required.

Over the course of the development of the ``asap`` modules, a certain usage
model became stronger, and that usage model (described below) resulted in the
current design of the logging interface.

* We encapsulate the underlying frameowrk logger into a smaller interface
  that just offeres what is needed. This is represented by
  the :class:`asap::logging::Logger` class.

* Loggers within an application should be well known, documented and referring
  to well known submodules. Therefore, we made the explicit choice to identify
  all loggers in the application with a statically defined ID part of the
  :enum:`asap::logging::Id` enum. (TODO: change the logging interface in the
  ``common`` module to allow the application to specify application specific
  logger id's without needing to fork the common submodule.)

* The use of well know loggers, identified by statically defined id's, allows
  for most of the boilerplate code to be written only once and totally hidden
  from the application. This includes formatters, sinks, filtering, etc... and
  this is mostly provided through the :class:`asap::logging::Registry` class.

* Logging is mostly done and is required within the implementation code of
  classes. We offer a mixin interface :class:`asap::logging::Loggable` that
  when inherited from, adds support for logging from anywhere within the
  class's implementation by simply using :c:macro:`ASLOG` macro.

* An advanced use case requiring the logging sink to be changed at runtime (
  something not allowed by spdlog) was identified and handled through the
  introduction of a special purpose sink implemented by the
  :class:`asap::logging::DelegatingSink` class.

Classes
-------

* :class:`asap::logging::Logger`
* :class:`asap::logging::Registry`
* :class:`asap::logging::Loggable`
* :class:`asap::logging::DelegatingSink`

.. doxygenclass:: asap::logging::Logger
   :members:

.. doxygenclass:: asap::logging::Registry
   :members:

.. doxygenclass:: asap::logging::Loggable
   :members:

.. doxygenclass:: asap::logging::DelegatingSink
   :members:


Helper macros
-------------

A set of convenience macros are provided and in 99% of the cases, these macros
will fulfill the logging requirements.

.. doxygendefine:: ASLOGGER

.. doxygendefine:: ASLOG

.. doxygendefine:: ASLOG_CHECK_LEVEL

.. doxygendefine:: ASFLUSH_LOG

.. doxygendefine:: ASLOG_TO_LOGGER

.. doxygendefine:: ASLOG_MISC
