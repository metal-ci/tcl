# tclbind 

```cpp
TCLBIND_PACKAGE(GetStarted, "1.0", mod)
{
  tclbind::create_command(mod, "hello")
    .add_function(
        +[](boost::core::string_view name)
        {
          std::cout << "Hello " << name << "!" << std::endl;
        });
}
```

```tcl
load ./libgetstarted[info sharedlibextension]

hello [lindex $argv 1]
```

This C++17 library makes it easy to extend tcl with C++ or vice versa. It requires boost and TCL.

Read the docs [here](doc/tcl.adoc).

## Why TCL?

Tcl is a unique combination of a full script language with a command language.
You can execute other programs with `tclsh`, like this:

```tcl
g++ hello_world.cpp
```

while at the same time load and execute arbitrary C or C++ code, e.g.:

```tcl
load ./libbuildhelpers[info sharedlibextension]

g++ [list-sources]
```

where `list-sources` is a command defined in C++.

Since tcl can be embedded in C++ as well, it is also a great starting point for interactive console applications;
and because of the syntax it can be used to create DSLs that do not look and feel like Tcl.
`redis` and `sqlite` started out that way.


## License

Distributed under the [Boost Software License, Version 1.0](http://boost.org/LICENSE_1_0.txt).
