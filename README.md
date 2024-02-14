# ECS & SFML Template

This code was created during a university project to provide a solid game development basis. It provides an ECS which may be used via the coordinator class engine. Please refer to the docs which can be generated via running `doxygen Doxyfile` which requires doxygen to be installed. Also see the `main.cc` file for example usage.

## Base requirements

- clang, gcc or msvc
- cmake
- conan2 (requires python3 and pip3)

## Initial setup

If you haven't installed conan package manager before:

```sh
pip install conan
```

then configure conan using:

```sh
conan profile detect --force
```

read the output it may hint at missing requirements like a compiler. If there is
something missing install it and repeat this step.

Install all dependencies and create cmake files for the `Debug` target:

```sh
conan install . --build=missing -c tools.system.package_manager:sudo=True -c tools.system.package_manager:mode=install -s build_type=Debug
```

Now we may build the `Debug` target:

```sh
conan build . -s build_type=Debug
```

The executable should now be located in one of those locations:

Macos, Linux:

```sh
build/Debug/manhattan_bin
```

Windows:

```sh
.\build\Debug\manhattan_bin.exe
```

If you want to build the `Release` target replace `Debug` with `Release` in both
conan commands and re-run them. Afterwards navigate to the `Release` folder
and run the application.

Note: To run tests just run `ecs_test`.
For example on Macos, Linux:

```sh
build/Debug/manhattan_test
```
