# 🧪 `malloc_tester` – A Simple Memory Allocation Tester for C Programs

`malloc_tester` is a preloadable shared object that hooks into your C application's memory allocation calls (only `malloc` for now) for testing purposes. It allows you to simulate allocation failures, memory limits, and observe allocation patterns in real time—perfect for testing error-handling logic.

---

## 📦 Requirements

To make the most of `malloc_tester`, compile your target program with debug symbols:

```bash
gcc -rdynamic -g -o target_program target_program.c
```

If you're using an external static library (like MiniLibX), wrap it inside a shared object:

```bash
gcc -shared -o libwrapper.so -Wl,--whole-archive lib_to_wrap.a -Wl,--no-whole-archive
```

Update your linker flags accordingly:

```makefile
LFLAGS = -L. -lmlxwrapper -Wl,-rpath,.
# or
LFLAGS = -Llib_folder -lmlxwrapper -Wl,-rpath,lib_folder
```

This avoids interfering with internal memory management in those libraries.

---

## Usage

### 🚀 Quickstart with Script

Run your target program normally:

```bash
./run_tester.sh ./target_program [args...]
```

Or debug with GDB:

```bash
./run_tester.sh gdb ./target_program [args...]
```

---

### 🛠️ Manual Setup

1. **Compile the Tester:**

```bash
gcc -fPIC -shared -o malloc_tester.so malloc_tester.c -ldl -g
```

2. **Run the Target Program:**

```bash
LD_PRELOAD=./malloc_tester.so TARGET_BIN=./target_program ./target_program
```

3. **Run in GDB:**

```bash
LD_PRELOAD=./malloc_tester.so TARGET_BIN=./target_program gdb ./target_program
```

---

## ⚙️ Runtime Configuration

You can modify tester parameters live during a GDB session:

```gdb
set malloc_cfg.max_calls = <int>
set malloc_cfg.fail_percent = <int>
```

### Available Configuration Options:

| Variable           | Description                                         | Default     |
|--------------------|-----------------------------------------------------|-------------|
| `max_calls`        | Max allocation calls allowed (`-1` = unlimited)     | -1          |
| `max_memory`       | Max bytes allowed to be allocated (`-1` = unlimited)| -1          |
| `fail_percent`     | Percent chance that an allocation will fail (0–100) | 10          |
| `rejected_symbols` | Ignore allocations from specific function names     | *(empty)*   |
| `print_log`        | Print outup to stderr                               | true        |

## 📜 License

This project is distributed under the DoTheFYouWant License. See `LICENSE` for more details.
