WINDOWS EXE WITHOUT INSTALLING VISUAL STUDIO LOCALLY

This workflow builds the project on a hosted Windows 2022 runner.
It does NOT produce a full graphical Vulkan game: Vulkan is disabled in this
first build because the current project is a headless engine foundation.

Steps:
1. Create a GitHub account and a new empty repository.
2. Upload the contents of this project folder to the repository (keep .github).
3. Open the repository's Actions tab.
4. Select "Build Windows EXE" and click "Run workflow", or push to main.
5. Wait for the workflow to finish.
6. Open the completed workflow run and download the artifact:
   ULTIMATE_FPS-Windows-x64
7. Extract it. ULTIMATE_FPS.exe is the Windows executable.

The workflow runs CMake configure, Release build, and CTest on Windows.
