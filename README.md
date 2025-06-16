# 🧪 `malloc_tester` – A Simple Memory Allocation Tester for C Programs

`malloc_tester` is a preloadable shared object that hooks into your C application's memory allocation calls (only `malloc` for now) for testing purposes. It allows you to simulate allocation failures, memory limits, and observe allocation patterns in real time—perfect for testing error-handling logic.

---

## 📦 Requirements

To make the most of `malloc_tester`, compile your target program with debug symbols:

```bash
cc -rdynamic -g -o target_program target_program.c
```
```makefile
CFLAGS += -g
LDFLAGS += -rdynamic
```

If you're using an external static library (like MiniLibX), wrap it inside a shared object:

```bash
cd lib
gcc -shared -o libwrapper.so -Wl,--whole-archive libmlx42.a -Wl,--no-whole-archive
```

Update your linker flags accordingly:

```makefile
#make sure to specify the right folder
LFLAGS += -Lfolder -lwrapper -Wl,-rpath,folder
#for exemple if the wrapper is in the project root folder
LFLAGS += -L. -lwrapper -Wl,-rpath,.
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
