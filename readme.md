# boost.tcl 

```cpp
BOOST_TCL_PACKAGE(GetStarted, "1.0", mod)
{
  boost::tcl::create_command(mod, "hello")
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


This C++17 library makes it easy to extend tcl with C++ or vice versa.


Read the docs [here](doc/tcl.adoc).

## License

Distributed under the [Boost Software License, Version 1.0](http://boost.org/LICENSE_1_0.txt).