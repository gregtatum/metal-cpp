#!/usr/bin/env node

/**
 * This file is in charge of live-reloading the example. It is used by the make
 * file via `make watch EXAMPLE=bunny`.
 */

const watch = require("watch");
const path = require("path");
const fs = require("fs");
const { spawn, spawnSync, exec, execSync } = require("child_process");
const rootPath = path.join(__dirname, "..");
const example = getExample();
let exampleSubProcess;
let isClosingSubProcess = false;
let keepExampleClosed = false;

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

clearConsole();
buildAndRun();
watchFiles();
listenToKeyboard();

process.on("SIGINT", function (...args) {
  console.log("Closing the example");
  exampleSubProcess.kill();
  process.exit(0);
});

function buildAndRun() {
  if (build()) {
    if (keepExampleClosed) {
      printShortcuts();
      console.log('Hit "r" to re-launch the example');
    } else {
      runExample();
    }
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
  if (exampleSubProcess) {
    isClosingSubProcess = true;
    const success = exampleSubProcess.kill();
    if (!success) {
      console.error("Unable to close the example " + example);
      process.exit(1);
    }
    exampleSubProcess = null;
  }
}

function listenToKeyboard() {
  process.stdin.setRawMode(true);
  process.stdin.resume();
  process.stdin.setEncoding("utf8");
  process.stdin.on("data", function (key) {
    switch (key) {
      case "c":
        try {
          clearConsole();
          console.log("🧹 Cleaning all of the C++ files.");
          console.log("");
          execSync(`make clean-cpp`, { cwd: rootPath, stdio: "inherit" });
        } catch (error) {
          console.error("🛑 Could not clean the C++ files. " + example);
          return false;
        }
        closeAndRebuild();
        break;
      case "a":
        clearConsole();
        console.log("🧹 It's time for a fresh start");
        console.log("");
        try {
          execSync(`make clean`, { cwd: rootPath, stdio: "inherit" });
        } catch (error) {
          console.error("🛑 Could not clean the files. " + example);
          return false;
        }
        closeAndRebuild();
        break;
      case "r":
        closeSubprocess();
        clearConsole();
        runExample();
        keepExampleClosed = false;
        break;
      case "l":
        closeSubprocess();
        clearConsole();
        runExample({ LOG_SHADER_CALLS: "1" });
        keepExampleClosed = false;
        break;
      case "k":
        closeSubprocess();
        printShortcuts();
        keepExampleClosed = true;
        break;
      case "q":
      case "\u0003":
        closeSubprocess();
        process.exit();
      case "0":
      case "1":
      case "2":
      case "3":
      case "4":
      case "5":
      case "6":
      case "7":
      case "8":
      case "9": {
        exampleSubProcess.stdin.write(key + "\n");
      }
    }
  });
  printShortcuts();
}

function printShortcuts() {
  console.log("");
  console.log("-------------------------");
  console.log("| Keyboard shortcuts:   |");
  console.log("-------------------------");
  console.log("  c - Clean the C++ files");
  console.log("  a - Clean all the files");
  console.log("  r - Restart the example");
  console.log("  l - Log shaders");
  console.log("  k - Kill process");
  console.log("  q - Quit");
  console.log("  1-9 - Do a GPU trace of a certain length");
  console.log("");
}

function runExample(env = {}) {
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
    stdio: [
      // stdin
      "pipe",
      // stdout
      "inherit",
      // stderr
      "inherit",
    ],
    env: { ...metalValidation, ...env },
  });

  exampleSubProcess.stdin.setEncoding("utf-8");

  exampleSubProcess.on("close", (code, signal) => {
    if (isClosingSubProcess) {
      isClosingSubProcess = false;
    } else {
      if (code || signal) {
        console.log(`Signal: ${signal}`);
        console.log(`code: ${code}`);
        console.log();
        console.log(
          `Example closed, hit "r" to re-launch it, or "q" then run:`
        );
        console.log(`lldb ./bin/${example}`);
      } else {
        console.log();
        console.log(`Example closed, hit "r" to re-launch it.`);
      }
      exampleSubProcess = null;
      isClosingSubProcess = false;
      keepExampleClosed = true;
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

function handleFileChange(fileName) {
  clearConsole();
  console.log("🙈 File change detected", fileName);
  closeAndRebuild();
}

function closeAndRebuild() {
  if (exampleSubProcess) {
    closeSubprocess();
  }
  buildAndRun();
}

function watchFiles() {
  const dirs = [path.join(__dirname, "../src")];
  const files = [path.join(__dirname, "../Makefile")];
  const options = { interval: 0.5 };

  for (const fileName of files) {
    fs.watch(fileName, options, (eventType, maybeFileName) => {
      handleFileChange(fileName);
    });
  }
  for (const dir of dirs) {
    // Interval is in seconds.
    watch.watchTree(dir, options, (...args) => {
      if (!args[1]) {
        const [allStats] = args;
        // This is the first run, do nothing.
      } else {
        const [fileName, prevStat, currState] = args;
        handleFileChange(fileName);
      }
    });
  }
  console.log("👀 Watching the tree");
}

function watchForGpuTraceFiles() {
  const dir = path.join(__dirname, "../bin");
  const options = { interval: 0.5 };

  fs.watch(dir, (eventType, filename) => {
    console.log({ eventType, filename });
  });

  // Interval is in seconds.
  watch.watchTree(dir, options, (...args) => {
    if (!args[1]) {
      const [allStats] = args;
      // This is the first run, do nothing.
    } else {
      const [fileName, prevStat, currState] = args;
      console.log({ fileName });
      if (fileName.endsWith(".gputrace")) {
        console.log("gputrace discovered, opening it" + fileName);
        // execSync("open + " + fileName);
      }
    }
  });
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

function clearConsole() {
  process.stdout.write("\033[2J\033[3J\033[1;1H");
  // process.stdout.write('\x1Bc');
  // console.log('\033]50;ClearScrollback\a');
  // printf '\033[2J\033[3J\033[1;1H'
}
