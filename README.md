![Magnet](assets/banner.png)

- 🧲 Magnet is a minimalistic dependency manager for your next C++ project.
- 🏗 Currently, it is not production ready and thus should be used carefully.️

# Features

| Feature                                                                                      | Ready?          |
|----------------------------------------------------------------------------------------------|-----------------|
| CLI tool                                                                                     | ✅               |
| Global command `magnet`                                                                      | ⚠️ (macOS only) |
| Bootstrap a new C++ project with a single command                                            | ✅               |
| Fine tuning after project creation (customize README, LICENSE, .gitignore, etc.)             | ✅               |
| Automatically generate CMakeLists.txt files based on your dependencies and project structure | ✅               |
| Pull dependencies from GitHub                                                                | ✅               |
| Switch dependency branch                                                                     | ✅               |
| Remove installed dependencies                                                                | ✅               |
| Pull dependencies from GitLab                                                                | ❌               |
| Pull dependencies from BitBucket                                                             | ❌               |
| Pull dependencies from custom Git repositories                                               | ✅               |
| macOS support (Xcode generator)                                                              | ✅               |
| Linux support (Unix Makefiles generator)                                                     | ❌               |
| Windows support (Visual Studio generator)                                                    | ❌               |

# 🛠️ Installation

macOS ONLY (other platforms will be supported very soon™️):

1. Clone repo: `git clone https://github.com/futureblur/magnet.git --recursive`
2. Generate Xcode project files: `cd magnet/scripts && chmod +x ./Xcode.sh && ./Xcode.sh`
4. Open `magnet/Build/magnet.xcodeproj` and build `magnet` target: **⌘ + B**
5. Create `magnet` symlink: `chmod +x ./macOSInstall.sh && ./macOSInstall.sh`

# 🏛️ History

Let’s face it: managing your dependencies in a C++ project is a pain in the butt.
****
On one hand, you can let existing solutions like Conan or vcpkg handle that, and on the other hand, you could take it on
yourself and embrace the countless hours wasted on setting up a new project with your desired dependencies.

But here’s the problem: when you’re in the middle of creating a project and are searching for a solution, you don’t have
the time to learn new tech.

In fact, you’ll never have time to learn every new tool that probably isn’t worth dedicating your time to. You want
something that can be set up in minutes.

And here’s where Magnet comes into play.

# 💡 The Idea

We take a different approach. We like to simplify things. To a point where it’s not event possible anymore.

Magnet does not lock you in. You are completely in charge of your entire project. At all times.

Magnet is not a new build tool. Nor does it host packages in a dedicated cloud. Instead, we leverage existing,
battle-tested technologies to do the heavy lifting.

What you see, is what you get.

Installing Magnet for your system takes 60 seconds.
Pulling a dependency, 10 seconds.
Scaffolding a new project, 5 seconds.

Imagine what you could do will all of this free time now.
Say goodbye to unexpected setup errors: allowing you to focus on coding and innovating great products.