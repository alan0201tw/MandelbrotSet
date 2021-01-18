workspace "MandelbrotSet"
    architecture "x64"
    startproject "MandelbrotSet"

    configurations
    {
        -- only debug for this hobby project
        "Debug",
        "Release"
    }

-- variables
    -- cfg - configuration
outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "MandelbrotSet"
    location "."
    kind "ConsoleApp"
    language "C++"
    toolset "clang"
    staticruntime "off"

    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    
    files
    {
        "include/**.hpp",
        "src/**.cpp"
    }

    includedirs
    {
        "include",
        "vendor/include"
    }

    libdirs
    {
        "vendor/lib"
    }

    filter "system:linux"
        cppdialect "C++17"
        systemversion "latest"
        links
        {
            "GL",
            "glfw",
            "dl"
        }

    -- everything under this filter only applies to windows
    filter "system:windows"
        cppdialect "C++17"
        systemversion "latest"

        defines
        {
            "PLATFORM_WINDOWS",
            "_USE_MATH_DEFINES"
        }

        linkoptions { "opengl32.lib glfw3.lib" }

        -- links
        -- {
        --     "opengl32",
        --     "glfw3"
        -- }

    filter { "configurations:Debug" }
        symbols "On"
    
    filter { "configurations:Release" }
        optimize "On"