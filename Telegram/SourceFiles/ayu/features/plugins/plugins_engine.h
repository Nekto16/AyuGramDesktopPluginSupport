// This is the source code of AyuGram for Desktop.
//
// We do not and cannot prevent the use of our code,
// but be respectful and credit the original author.
//
// Copyright @Radolyn, 2026
#pragma once

#include <QString>

namespace AyuFeatures::Plugins {

void start();
[[nodiscard]] bool isPythonAvailable();
[[nodiscard]] QString pythonVersion();

} // namespace AyuFeatures::Plugins
