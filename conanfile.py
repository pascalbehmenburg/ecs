from conan import ConanFile
from conan.tools.cmake import CMake, cmake_layout


class ecsRecipe(ConanFile):
    name = "ecs"
    version = "1.0"
    package_type = "application"

    license = "MIT"
    author = ("Pascal Behmenburg")
    url = "<Package recipe repository url here, for issues about the package>"
    description = "ECS"
    topics = "ecs"

    settings = "os", "compiler", "build_type", "arch"
    generators = "CMakeToolchain", "CMakeDeps"
    exports_sources = "CMakeLists.txt", "src/*", "tests/*"

    def requirements(self):
        self.requires("sfml/2.6.1")
        self.requires("fmt/10.2.0")
        self.requires("gtest/1.14.0")
        self.requires("spdlog/1.13.0")

    def layout(self):
        cmake_layout(self)

    def build(self):
        cmake = CMake(self)
        cmake.configure()
        cmake.build()

    def package(self):
        cmake = CMake(self)
        cmake.install()
