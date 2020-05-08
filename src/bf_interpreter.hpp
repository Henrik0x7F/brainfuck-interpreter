#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include <string_view>


enum struct BF_Opcode : std::uint8_t
{
    INVALID,
    INC_CELL,
    JMP_FWD,
    JMP_BWD,
    PUT,
    GET,
    ZERO
};

namespace BF_Instr
{

struct op_t
{
    BF_Opcode op;
};

using cell_access_t = std::int16_t;

struct inc_cell_t : op_t
{
    std::uint8_t val;
    cell_access_t cell;
};

struct jmp_fwd_t : op_t
{
    cell_access_t cell;
    std::uint32_t dest;
};

using jmp_bwd_t = jmp_fwd_t;

struct zero_t : op_t
{
    cell_access_t cell;
};

struct put_t : op_t
{
    cell_access_t cell;
};

using get_t = put_t;

}

union bf_instr_t
{
    BF_Instr::op_t check;

    BF_Instr::inc_cell_t inc_cell;
    BF_Instr::jmp_fwd_t jmp_fwd;
    BF_Instr::jmp_bwd_t jmp_bwd;
    BF_Instr::zero_t zero;
    BF_Instr::put_t put;
    BF_Instr::get_t get;
};

struct BF_Interpreter
{
    bool load_code(const char* code, std::size_t len);

    bool execute_all();

    bool execute(std::uint32_t num_instr);

    [[nodiscard]]
    std::size_t code_size() const;

    [[nodiscard]]
    bool finished() const;

    void reset();

    [[nodiscard]]
    std::string disassemble() const;


    static constexpr std::size_t NUM_CELLS{3000};

private:
    bool generate_code(std::string_view code);

    template<bool EXECUTE_ALL>
    bool execute(std::uint32_t num_instr);

    std::vector<bf_instr_t> code_;

    struct
    {
        std::uint16_t cp{};
        std::uint32_t pc{};
        std::vector<char> cells;
    }bf_;
};
