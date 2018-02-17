#include "../obj_dir/Vxenowing.h"
#include "verilated_vcd_c.h"

#include <cinttypes>
#include <iostream>
using namespace std;

#include <Windows.h>

typedef struct
{
    uint32_t (*get_reset_n)();
    void (*set_reset_n)(uint32_t value);
    uint32_t (*get_clk)();
    void (*set_clk)(uint32_t value);

    uint32_t (*get_program_rom_addr)();
    void (*set_program_rom_addr)(uint32_t value);
    uint32_t (*get_program_rom_q)();
    void (*set_program_rom_q)(uint32_t value);

    uint32_t (*get_leds)();
    void (*set_leds)(uint32_t value);

    void (*eval)();
    void (*final)();
    void (*trace_dump)(uint64_t time);
} Env;

static Vxenowing *top = nullptr;
static VerilatedVcdC *trace = nullptr;

uint32_t get_reset_n()
{
    return top->reset_n;
}

void set_reset_n(uint32_t value)
{
    top->reset_n = value;
}

uint32_t get_clk()
{
    return top->clk;
}

void set_clk(uint32_t value)
{
    top->clk = value;
}

uint32_t get_program_rom_addr()
{
    return top->program_rom_addr;
}

void set_program_rom_addr(uint32_t value)
{
    top->program_rom_addr = value;
}

uint32_t get_program_rom_q()
{
    return top->program_rom_q;
}

void set_program_rom_q(uint32_t value)
{
    top->program_rom_q = value;
}

uint32_t get_leds()
{
    return top->leds;
}

void set_leds(uint32_t value)
{
    top->leds = value;
}

void eval()
{
    top->eval();
}

void final()
{
    top->final();
}

void trace_dump(uint64_t time)
{
    if (trace)
        trace->dump(time);
}

int main(int argc, char **argv)
{
    if (argc != 2 && argc != 3)
    {
        cout << "Invalid number of arguments" << endl;
        exit(1);
    }

    auto libName = argv[1];
    auto lib = LoadLibrary(libName);
    if (lib == NULL)
    {
        cout << "Failed to load library: " << libName << endl;
        exit(1);
    }
    auto run = (int32_t (*)(Env *))GetProcAddress(lib, "run");
    if (run == NULL)
    {
        cout << "Failed to get run proc address" << endl;
        exit(1);
    }

    top = new Vxenowing();

    if (argc == 3)
    {
        Verilated::traceEverOn(true);
        trace = new VerilatedVcdC();
        top->trace(trace, 99); // TODO: What is this param?
        trace->open(argv[2]);
    }

    Env env =
    {
        .get_reset_n = get_reset_n,
        .set_reset_n = set_reset_n,
        .get_clk = get_clk,
        .set_clk = set_clk,

        .get_program_rom_addr = get_program_rom_addr,
        .set_program_rom_addr = set_program_rom_addr,
        .get_program_rom_q = get_program_rom_q,
        .set_program_rom_q = set_program_rom_q,

        .get_leds = get_leds,
        .set_leds = set_leds,

        .eval = eval,
        .final = final,
        .trace_dump = trace_dump,
    };

    auto ret = run(&env);

    if (trace)
    {
        trace->close();
        delete trace;
    }

    delete top;

    return ret;
}