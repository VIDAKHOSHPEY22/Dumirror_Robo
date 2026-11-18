
# Contributing to Dumirror_Robo

Thank you for your interest in contributing to **Dumirror_Robo**.

Dumirror_Robo is an open-source Arduino library for building expressive robot faces and animations on SSD1306 128×64 OLED displays. Contributions from developers, hardware enthusiasts, designers, and documentation writers are welcome.

This guide outlines the contribution process, development requirements, coding conventions, and guidelines for submitting changes.

Whether you are reporting a bug, proposing an improvement, creating original artwork, testing hardware compatibility, or contributing code, your participation helps improve the project.

---

## Table of Contents

- [Code of Conduct](#code-of-conduct)
- [Ways to Contribute](#ways-to-contribute)
- [Development Setup](#development-setup)
- [Reporting Bugs](#reporting-bugs)
- [Suggesting Features](#suggesting-features)
- [Pull Request Process](#pull-request-process)
- [Adding a New Face](#adding-a-new-face)
- [Adding a New Animation](#adding-a-new-animation)
- [Code Style](#code-style)
- [Asset Guidelines](#asset-guidelines)
- [Commit Message Convention](#commit-message-convention)
- [License of Contributions](#license-of-contributions)

---

## Code of Conduct

All contributors are expected to communicate respectfully and constructively.

Dumirror_Robo aims to provide an inclusive and collaborative environment for makers and developers of all experience levels, from beginners exploring Arduino for the first time to experienced embedded systems developers.

Please:

- Be respectful of different perspectives and experience levels.
- Provide constructive feedback.
- Keep discussions focused on the project.
- Be patient when reviewing or responding to contributions.
- Avoid discriminatory, offensive, or harassing behavior.

Harassment, discrimination, and hostile conduct are not acceptable.

---

## Ways to Contribute

Contributions are not limited to source code. There are several ways to help improve Dumirror_Robo:

| Contribution | Description |
|---|---|
| Bug Reports | Identify unexpected behavior, compilation errors, or compatibility issues. |
| Feature Requests | Propose new functionality, expressions, animations, or API improvements. |
| Facial Artwork | Create original facial expressions and visual assets. |
| Animations | Design new frame-based animations. |
| Documentation | Improve explanations, installation instructions, examples, and technical references. |
| Hardware Testing | Test the library on different boards and report verified results. |
| Hardware Demonstrations | Share projects, photographs, videos, and wiring configurations. |
| Translations | Help make documentation accessible to a wider audience. |
| Code Contributions | Improve performance, maintainability, memory efficiency, and compatibility. |

Contributions of all sizes are valuable.

---

## Development Setup

### Prerequisites

Before making changes, ensure that you have the following:

- Arduino IDE 1.8 or later, or PlatformIO.
- A compatible C++ toolchain.
- Adafruit GFX Library.
- Adafruit SSD1306 Library.
- A compatible microcontroller for hardware testing.
- An SSD1306 128×64 OLED display for display-related testing.

The library declares compatibility with AVR, ESP32, ESP8266, and RP2040 architectures. Actual compatibility should be verified against the relevant board core and development environment.

### 1. Fork the repository

Create your own fork of the repository:

https://github.com/VIDAKHOSHPEY22/Dumirror_Robo

### 2. Clone your fork

Replace `YOUR_USERNAME` with your GitHub username.

```bash
git clone https://github.com/YOUR_USERNAME/Dumirror_Robo.git
cd Dumirror_Robo
```

### 3. Configure the upstream repository

Add the original repository as an upstream remote:

```bash
git remote add upstream https://github.com/VIDAKHOSHPEY22/Dumirror_Robo.git
```

This allows you to synchronize your fork with the latest project changes.

### 4. Create a feature branch

Use a descriptive branch name that reflects the purpose of your contribution.

```bash
git checkout -b feat/add-happy-face
```

Other examples:

```text
fix/animation-timing
docs/update-installation-guide
feat/add-new-animation
test/esp32-compatibility
```

### 5. Make and test your changes

Keep changes focused and consistent with the existing project architecture.

Before submitting, review the relevant source files, examples, documentation, and tests.

---

## Reporting Bugs

Before creating a new issue, search the existing issue tracker to check whether the problem has already been reported.

When submitting a bug report, provide as much relevant technical information as possible.

Include:

- **Microcontroller:** Exact board model, such as Arduino Uno or ESP32-S3.
- **Board Package:** Installed board core and version.
- **Development Environment:** Arduino IDE or PlatformIO version.
- **Display:** OLED controller, resolution, and I2C address.
- **Library Version:** The version of Dumirror_Robo being used.
- **Reproduction Steps:** A clear sequence of steps that triggers the problem.
- **Expected Behavior:** What you expected the library to do.
- **Actual Behavior:** What happened instead.
- **Error Output:** Relevant compiler messages, runtime logs, or serial output.
- **Visual Evidence:** Photographs or short videos when the issue involves display rendering or animation playback.

Whenever possible, include a minimal reproducible sketch.

A focused example makes it easier to identify the source of a problem and reduces the time required for investigation.

---

## Suggesting Features

Feature proposals are welcome, particularly those that improve usability, extensibility, performance, and hardware compatibility.

A useful feature request should explain:

1. **Problem:** What limitation or requirement does the proposed feature address?
2. **Proposed Solution:** What functionality should be introduced?
3. **Use Case:** How would makers or developers benefit from it?
4. **API Considerations:** How might the feature integrate with the existing library interface?
5. **Alternatives:** Are there other approaches worth considering?

For proposed facial expressions or animations, include a sketch, reference image, or brief description of the intended behavior.

For larger changes, consider opening an issue first to discuss the proposed design before implementation.

---

## Pull Request Process

All code and documentation changes should be submitted through GitHub Pull Requests.

Follow these guidelines:

1. Keep each pull request focused on a specific feature, fix, or improvement.
2. Reference related issues when applicable (for example, `Closes #42`).
3. Provide a clear description of the problem and the changes introduced.
4. Include relevant test results.
5. Test on physical hardware when available. Simulator-based validation, such as Wokwi, may be used where appropriate.
6. Update documentation and examples when public APIs or library behavior change.
7. Include preview images or GIFs for new facial expressions and animations.
8. Identify any compatibility or memory-related implications.
9. Respond to review feedback and update the pull request when necessary.

Small, well-scoped pull requests are easier to review, test, and maintain.

### Pre-submission Checklist

Before opening a pull request, verify the following:

- [ ] The code follows the existing project conventions.
- [ ] The changes compile for the relevant target architecture.
- [ ] Existing functionality and examples have been checked.
- [ ] New faces and animations are registered correctly in `src/faces.def`.
- [ ] Required preview assets have been added.
- [ ] Documentation has been updated where necessary.
- [ ] Memory usage and non-blocking behavior have been considered.
- [ ] Relevant tests have been executed.
- [ ] Commit messages follow the project's convention.

---

## Adding a New Face

Dumirror_Robo uses a centralized registry to organize facial expressions.

To contribute a new static face, follow the existing asset and registration conventions.

### 1. Create the bitmap header

Create a new header file under:

```text
src/faces/face_happy.h
```

The bitmap should use the format expected by the library's existing face assets.

Tools such as [image2cpp](https://javl.github.io/image2cpp/) can be used to convert monochrome images into C/C++ bitmap arrays.

### 2. Include the new header

Add the corresponding include to the main library header, following the existing include structure.

```cpp
#include "faces/face_happy.h"
```

### 3. Register the face

Add an entry to:

```text
src/faces.def
```

Follow the existing static face registration convention:

```cpp
X(FACE_HAPPY, FACE_HAPPY_ID, "happy")
```

Ensure that the identifier, bitmap reference, and registered name are consistent with the library's implementation.

### 4. Add a preview image

Provide a preview image in the project's image directory:

```text
images/oled-128x64-happy.png
```

The preview should accurately represent how the expression appears on the OLED display.

### 5. Update the documentation

If applicable, update the face gallery, expression reference, and related documentation.

---

## Adding a New Animation

Animations in Dumirror_Robo consist of individual bitmap frames organized into a defined playback sequence.

A new animation should follow the existing animation implementation and registration conventions.

### 1. Create the animation header

Add a new animation header under:

```text
src/faces/anim_happy.h
```

### 2. Define the animation data

Provide the required animation components according to the existing format:

- Frame bitmap data.
- Frame sequence.
- Frame count.
- Frame durations or timing configuration.
- Playback and looping configuration.

Keep frame dimensions and bitmap formats consistent with the existing animation assets.

### 3. Register the animation

Add the animation to:

```text
src/faces.def
```

Use the existing `X_ANIM(...)` registration format.

Ensure that all identifiers, references, and animation metadata match the implementation.

### 4. Add a GIF preview

Provide a preview under:

```text
images/animations/happy.gif
```

The preview should demonstrate the intended animation clearly.

### 5. Validate the animation

Before submitting, verify:

- Frame order and consistency.
- Playback duration.
- Looping behavior.
- Animation registration.
- Correct rendering on the target display.
- Compatibility with the library's non-blocking update mechanism.

---

## Code Style

Contributions should follow the conventions already established in the repository.

General guidelines:

- Use consistent four-space indentation.
- Prefer descriptive names for variables, functions, and types.
- Keep functions focused and reasonably small.
- Avoid unnecessary complexity.
- Document non-obvious implementation details.
- Keep changes compatible with the project's supported development environments.
- Avoid adding external dependencies unless there is a clear technical justification.

### C++ Guidelines

When contributing C++ code:

- Prefer `const` and `constexpr` where appropriate.
- Use `PROGMEM` for bitmap storage on supported architectures.
- Avoid unnecessary dynamic memory allocation, particularly in frequently executed code paths.
- Preserve non-blocking animation behavior.
- Consider RAM and flash constraints on resource-limited boards.
- Avoid architecture-specific assumptions in shared code.
- Use conditional compilation when platform-specific functionality is required.

Changes to memory management, bitmap storage, or animation scheduling should include appropriate validation.

---

## Asset Guidelines

New facial expressions and animations should meet the following requirements.

| Property | Requirement |
|---|---|
| Face Resolution | 128×64 pixels |
| Face Color Depth | Monochrome, 1-bit |
| Static Image Format | PNG |
| Animation Format | GIF for previews |
| Naming Convention | Lowercase with underscores |
| Asset Quality | Clear, consistent, and suitable for OLED rendering |
| Originality | Original artwork or assets with compatible licenses |

### Naming Examples

Static expression:

```text
oled-128x64-happy.png
```

Animation preview:

```text
happy.gif
```

### Artwork and Licensing

Only submit artwork that you created yourself or are legally permitted to distribute under a license compatible with the project.

Do not include proprietary, copyrighted, or copied assets without appropriate authorization.

Keep preview GIFs reasonably small to ensure that documentation and repository pages load efficiently.

---

## Commit Message Convention

Use clear, concise, and action-oriented commit messages.

The project follows a conventional prefix-based format:

```text
feat: add happy facial expression
fix: correct sneeze animation timing
docs: clarify ESP32 I2C configuration
refactor: simplify face registry lookup
test: add animation playback test
chore: update development tools
```

### Commit Prefixes

| Prefix | Purpose |
|---|---|
| `feat` | New features, faces, or animations |
| `fix` | Bug fixes and corrections |
| `docs` | Documentation changes |
| `refactor` | Internal restructuring without intended behavior changes |
| `test` | Test additions or modifications |
| `chore` | Maintenance, tooling, and dependency updates |

Use one primary prefix per commit and describe the change as specifically as possible.

---

## License of Contributions

By submitting a contribution to Dumirror_Robo, you agree that your contribution will be distributed under the project's **MIT License**.

You confirm that:

- You are the original author of the submitted work, or
- You have obtained the necessary permissions and rights to submit the work under the MIT License.

You are responsible for ensuring that contributed code, artwork, documentation, and other assets do not infringe third-party intellectual property rights.

Contributions must not contain proprietary material or assets that cannot legally be redistributed under the project's license.

For the complete licensing terms, refer to the repository's [`LICENSE`](LICENSE) file.

---

## Thank You

Thank you for taking the time to contribute to Dumirror_Robo.

Every contribution, whether it is a bug report, a new expression, an animation, documentation improvement, or a hardware compatibility report, helps make the library more useful and accessible to the maker community.

**Build, experiment, improve, and give your robots a little more personality.**

<p align="center">
  <b>Dumirror_Robo — Open Source, Built for Makers.</b>
  <br>
  <a href="https://github.com/VIDAKHOSHPEY22/Dumirror_Robo">
    GitHub Repository
  </a>
  ·
  <a href="https://vidakhoshpey22.github.io/Dumirror_Robo/">
    Live Gallery
  </a>
</p>
