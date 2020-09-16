#pragma once

/**
 * This file controls the dependency order for loading in files. Note
 * that each include is separated by a comment to keep clang-format from
 * re-ordering the files.
 */

// This is the main dependency.
#include "mtlpp/mtlpp.hpp"
// Macros can be used everywhere.
#include "viz/macros.h"
// Assertions are general-purpose.
#include "viz/assert.h"
// Includes the Cocoa bindings to start the app.
#include "viz/cocoa-app.h"
// Includes helpful utils to work with the metal API, especially with Objective
// C work.
#include "viz/metal.h"
// Includes utils for .metal shader files.
#include "viz/shader-utils.h"
// C++ interfaces to the GLKMath C library. Note that the name is viz-math.h
// and not just math.h, as it makes some include resolutions ambiguous
// otherwise.
#include "viz/math.h"
