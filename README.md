# File Integrity Monitor (FIM)

A lightweight command-line file integrity monitor written in C++. It detects when files in a directory have been modified, deleted, or added by comparing cryptographic hashes against a saved baseline — the same core concept used by production tools like Tripwire and AIDE.

## What it does

1. **Take a baseline** — scans a directory recursively, computes a SHA-256 hash for every file, and saves the result to disk.
2. **Scan later** — re-hashes the directory and compares it against the saved baseline.
3. **Report differences** — flags any file that was `[MODIFIED]`, `[DELETED]`, or is `[NEW]` since the baseline was taken.

Because even a single changed byte in a file produces a completely different SHA-256 hash, this approach reliably catches any tampering — no matter how small.

## Tech stack

- **C++17** — for `std::filesystem` directory traversal
- **OpenSSL (`libcrypto`)** — for SHA-256 hashing
- Plain text file storage for the baseline (`path,hash` per line) — no database required

## Build

Requires `g++` and OpenSSL development headers.

```bash
# Install OpenSSL dev headers (Debian/Ubuntu/Kali)
sudo apt update
sudo apt install libssl-dev

# Compile
g++ -std=c++17 hash_test.cpp -o hash_test -lcrypto
```

## Usage

**Take a baseline snapshot of a directory:**

```bash
./hash_test --baseline <folder>
```

This hashes every file in `<folder>` and saves the results to `baseline.txt`.

**Scan and compare against the saved baseline:**

```bash
./hash_test --scan <folder>
```

This re-hashes `<folder>` and compares it against `baseline.txt`, printing any differences.

## Example

```bash
$ ./hash_test --baseline .
Baseline saved for: .

$ echo "// tampered" >> somefile.cpp

$ ./hash_test --scan .
Comparing...
[MODIFIED] ./somefile.cpp
Done.
```

A clean scan (no changes since the baseline) prints `Comparing...` → `Done.` with nothing flagged in between.

## How it works

- `hashFile()` — reads a file's raw bytes and computes its SHA-256 hash using OpenSSL's `EVP_Digest` API, returning it as a hex string.
- `scanDirectory()` — recursively walks a directory with `std::filesystem::recursive_directory_iterator`, hashing every regular file (skipping the tool's own output files) and returning a `path → hash` map.
- `saveBaseline()` / `loadBaseline()` — write/read that map to/from `baseline.txt`, giving the tool persistent state across separate runs.
- `compareBaseline()` — diffs the loaded baseline against a fresh scan to detect modified, deleted, and newly added files.

## Notes

- The tool excludes its own `baseline.txt` and compiled binary from scans, so it doesn't flag its own output as tampering.
- Requires running `--baseline` at least once before `--scan` will have anything to compare against.

## Possible future improvements

- Graceful error handling for missing baseline files or invalid paths
- Config file to exclude specific files/folders from scanning
- JSON output mode for integration with other tools
- Scheduled/automatic scanning
