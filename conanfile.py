from conans import ConanFile, tools
import platform


class ModuleConan(ConanFile):
    name = "Glim"
    description = ""
    settings = "os", "compiler", "build_type", "arch"
    generators = "cmake", "cmake_find_package_multi", "cmake_paths"

    def configure(self):
        del self.settings.compiler.cppstd

    def requirements(self):
        self.requires("gtest/1.14.0")
        self.requires("sdl/2.28.5")
        self.requires("glm/cci.20230113")
        self.options["sdl"].shared = True
        
    def imports(self):
        self.copy("*.dll", "./bin", "bin")