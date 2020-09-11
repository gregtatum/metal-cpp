#!/usr/bin/env node

/**
 * This file is in charge of live-reloading the example. It is used by the make file
 * via `make watch EXAMPLE=bunny`.
 */

const watch = require("watch");
const path = require("path");
const { spawn, spawnSync, exec, execSync } = require("child_process");
const rootPath = path.join(__dirname, "..");
const example = getExample();
let exampleSubProcess;
let isClosingSubProcess = false;

const metalValidation = {
  // Enables all shader validation tests.
  MTL_SHADER_VALIDATION: "1",
  // Validates accesses to device and constant memory.
  MTL_SHADER_VALIDATION_GLOBAL_MEMORY: "1",
  // Validates accesses to threadgroup memory.
  MTL_SHADER_VALIDATION_THREADGROUP_MEMORY: "1",
  // Validates that texture references are not nil.
  MTL_SHADER_VALIDATION_TEXTURE_USAGE: "1",
};

buildAndRun();
watchFiles();

process.on("SIGINT", function () {
  console.log("Closing the example");
  exampleSubProcess.kill();
  process.exit(0);
});

function buildAndRun() {
  if (build()) {
    runExample();
  }
}

function build() {
  console.log("🛠  Building " + example);
  try {
    execSync(`make ./bin/${example}`, { cwd: rootPath, stdio: "inherit" });
  } catch (error) {
    console.error("🛑 Could not build " + example);
    return false;
  }
  return true;
}

function closeSubprocess() {
  isClosingSubProcess = true;
  const success = exampleSubProcess.kill();
  if (!success) {
    console.error("Unable to close the example " + example);
    process.exit(1);
  }
}

function runExample() {
  if (exampleSubProcess) {
    throw new Error(
      "A subprocess was still running when launching another example."
    );
  }
  console.log("🚂 Running " + example);
  const args = [];
  exampleSubProcess = spawn(`./bin/${example}`, args, {
    cwd: rootPath,
    detached: true,
    stdio: "inherit",
    env: metalValidation,
  });

  exampleSubProcess.on("close", (code, signal) => {
    if (isClosingSubProcess) {
      isClosingSubProcess = false;
    } else {
      console.log("Example closed, stopped watching for changes.");
      process.exit(0);
    }
  });
}

// type Stats = {
//   dev: 16777221,
//   mode: 16877,
//   nlink: 11,
//   uid: 502,
//   gid: 20,
//   rdev: 0,
//   blksize: 4096,
//   ino: 28649300,
//   size: 352,
//   blocks: 0,
//   atimeMs: 1598967445722.0557,
//   mtimeMs: 1598967445656.3203,
//   ctimeMs: 1598967445656.3203,
//   birthtimeMs: 1597688474814.448,
//   atime: 2020-09-01T13:37:25.722Z,
//   mtime: 2020-09-01T13:37:25.656Z,
//   ctime: 2020-09-01T13:37:25.656Z,
//   birthtime: 2020-08-17T18:21:14.814Z
// }
function handleFileChange(...args) {
  if (!args[1]) {
    const [allStats] = args;
    // This is the first run, do nothing.
  } else {
    const [fileName, prevStat, currState] = args;
    console.log("🙈 File change detected", fileName);
    if (exampleSubProcess) {
      closeSubprocess();
      exampleSubProcess = null;
    }
    buildAndRun();
  }
}

function watchFiles() {
  const paths = [
    path.join(__dirname, "../src"),
    path.join(__dirname, "../examples"),
  ];
  for (const path of paths) {
    // Interval is in seconds.
    const options = { interval: 0.5 };
    watch.watchTree(path, options, handleFileChange);
  }
  console.log("👀 Watching the tree");
}

function getExample() {
  const example = process.argv[2];

  function error(msg) {
    console.error("Error: " + msg);
    console.error("Usage: ./scripts/watch.js bunny");
    process.exit(1);
  }

  if (!example) {
    error("No example given to be built.");
  }
  if (process.argv.length !== 3) {
    error("Only one argument may be given to this command.");
  }
  if (!example.match(/^[\w-_]+$/)) {
    error("Examples should only be alphanumeric with dashes or underscores.");
  }
  return example;
}
