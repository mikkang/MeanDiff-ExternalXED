#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "xed/xed-interface.h"

enum Arch
{
    X86,
    X64,
    DEFAULT,
};

struct CmdOpt
{
    char *insn;
    size_t size;
    enum Arch arch;
};

enum Type
{
    BAD,
};

void help(char *progName)
{
    printf("[Usage] %s [Options...]", progName);
}

bool sanitize(char *insn)
{
    int i;

    if (strlen(insn) % 2 != 0) return false;

    for (i = 0; i < strlen(insn); i++)
    {
        if (!((insn[i] >= '0' && insn[i] <= '9') ||
                    (insn[i] >= 'a' && insn[i] <= 'f')))
        {
            return false;
        }
    }

    return true;
}

struct CmdOpt *parse(int argc, char **argv)
{
    struct CmdOpt *option = calloc(1, sizeof(struct CmdOpt));
    int opt;

    option->insn = NULL;
    option->size = 0;
    option->arch = DEFAULT;

    while (1)
    {
        opt = getopt(argc, argv, "i:s:a:");
        if (opt == -1) break;

        switch (opt)
        {
            case 'i':
                if (option->insn != NULL)
                {
                    free(option->insn);
                    free(option);
                    exit(-1);
                }
                option->insn = strdup(optarg);
                if (!sanitize(option->insn))
                {
                    free(option->insn);
                    free(option);
                    exit(-1);
                }
                break;
            case 's':
                if (option->size != 0)
                {
                    free(option->insn);
                    free(option);
                    exit(-1);
                }
                option->size = atoi(optarg);
                if (option->size == 0)
                {
                    free(option->insn);
                    free(option);
                    exit(-1);
                }
                break;
            case 'a':
                if (option->arch != DEFAULT)
                {
                    free(option->insn);
                    free(option);
                    exit(-1);
                }
                if (!strcmp(optarg, "x86"))
                {
                    option->arch = X86;
                }
                else if (!strcmp(optarg, "x64"))
                {
                    option->arch = X64;
                }
                else
                {
                    free(option->insn);
                    free(option);
                    exit(-1);
                }
                break;
            default:
                help(argv[0]);
                free(option->insn);
                free(option);
                exit(-1);
        }
    }

    return option;
}

int get_bytes(char *arg, xed_uint_t size, xed_uint8_t *insn)
{
    xed_uint_t i;
    xed_uint_t len = (xed_uint_t) strlen(arg) / 2;
    char *ptr;

    if (size != len) return -1;

    for (i = 0; i < len; i++)
    {
        sscanf(ptr, "%2hhx", &insn[i]);
        ptr += 2;
    }

    return 0;
}

bool chk_valid(enum Arch arch, xed_decoded_inst_t *xedd, xed_uint8_t *insn,
        xed_uint_t size)
{
    xed_machine_mode_enum_t mmode;
    xed_address_width_enum_t stack_addr_width;
    xed_error_enum_t xed_error;

    if (arch == X86)
    {
        mmode = XED_MACHINE_MODE_LEGACY_32;
        stack_addr_width = XED_ADDRESS_WIDTH_32b;
    }
    else
    {
        mmode = XED_MACHINE_MODE_LONG_64;
        stack_addr_width = XED_ADDRESS_WIDTH_64b;
    }

    xed_decoded_inst_zero(xedd);
    xed_decoded_inst_set_mode(xedd, mmode, stack_addr_width);
    xed_error = xed_decode(xedd, insn, size);

    return xed_error == XED_ERROR_NONE;
}

void get_type(struct CmdOpt *option, enum Type *t)
{
    xed_uint8_t insn[XED_MAX_INSTRUCTION_BYTES] = {0, };
    xed_decoded_inst_t xedd;
    xed_uint_t size = (xed_uint_t) option->size;
    xed_uint_t i;
    bool valid = false;

    xed_tables_init();

    if (get_bytes(option->insn, (xed_uint_t) option->size, insn) < 0) return;

    for (i = 0; i < XED_MAX_INSTRUCTION_BYTES - size; i++)
    {
        if (chk_valid(option->arch, &xedd, insn, size + i))
        {
            size = size + i;
            valid = true;
            break;
        }
    }

    if (valid)
    {

    }

    return;
}

int main(int argc, char **argv)
{
    struct CmdOpt *option = parse(argc, argv);
    enum Type t = BAD;

    get_type(option, &t);

    return t;
}