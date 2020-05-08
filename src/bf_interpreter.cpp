#include "bf_interpreter.hpp"

#include <algorithm>
#include <cstdio>
#include <iostream>
#include <sstream>
#include <stack>


template<bool EXECUTE_ALL>
bool BF_Interpreter::execute(std::uint32_t num_instr)
{
    auto cond{[num_instr, this](auto cur) -> bool
    {
        if constexpr(EXECUTE_ALL)
            return (bf_.pc < code_.size());
        return (cur < num_instr) && (bf_.pc < code_.size());
    }};

    auto inc{[](auto& cur) -> std::uint32_t
    {
        if constexpr(EXECUTE_ALL)
            return 0;
        return ++cur;
    }};

    for(std::uint32_t i{}; cond(i); inc(i))
    {
        auto instr{code_[bf_.pc]};

        switch(instr.check.op)
        {
        case BF_Opcode::INC_CELL:
            bf_.cells[bf_.cp + instr.inc_cell.cell] += instr.inc_cell.val;
            ++bf_.pc;
            break;
        case BF_Opcode::JMP_FWD:
            bf_.cp += instr.jmp_fwd.cell;
            if(bf_.cells[bf_.cp] != 0)
            {
                ++bf_.pc;
                break;
            }
            bf_.pc = instr.jmp_fwd.dest;
            break;
        case BF_Opcode::JMP_BWD:
            bf_.cp += instr.jmp_bwd.cell;
            if(bf_.cells[bf_.cp] == 0)
            {
                ++bf_.pc;
                break;
            }
            bf_.pc = instr.jmp_bwd.dest;
            break;
        case BF_Opcode::ZERO:
            bf_.cells[bf_.cp + instr.zero.cell] = 0;
            ++bf_.pc;
            break;
        case BF_Opcode::PUT:
            std::putchar(static_cast<int>(bf_.cells[bf_.cp + instr.put.cell]));
            ++bf_.pc;
            break;
        case BF_Opcode::GET:
            bf_.cells[bf_.cp + instr.zero.cell] = static_cast<char>(std::getchar());
            ++bf_.pc;
            break;
        case BF_Opcode::INVALID:
            return false;
        }
    }

    return true;
}

bool BF_Interpreter::load_code(const char* code, std::size_t len)
{
    bf_.cells.resize(NUM_CELLS);

    std::string clean_code(len, '\0');

    // Copy code, remove invalid chars
    auto end{std::remove_copy_if(code, code + len, clean_code.begin(), [](char c)
    {
        switch(c)
        {
        case '+':
        case '-':
        case '<':
        case '>':
        case '[':
        case ']':
        case '.':
        case ',':
            return false;
        default:
            return true;
        }
    })};

    clean_code.resize(std::distance(clean_code.begin(), end));

    return generate_code(clean_code);
}

bool BF_Interpreter::execute_all()
{
    return execute<true>(0);
}

bool BF_Interpreter::execute(std::uint32_t num_instr)
{
    return execute<false>(num_instr);
}

std::size_t BF_Interpreter::code_size() const
{
    return code_.size() * sizeof(bf_instr_t);
}

bool BF_Interpreter::finished() const
{
    return bf_.pc >= code_.size();
}

void BF_Interpreter::reset()
{
    bf_.cp = 0;
    bf_.pc = 0;
    bf_.cells.resize(NUM_CELLS);
    std::fill_n(bf_.cells.begin(), bf_.cells.size(), 0);
    code_.clear();
}

bool BF_Interpreter::generate_code(std::string_view code)
{
    std::stack<std::uint32_t> jmp_stack;
    std::int16_t cell_offset{};
    for(std::uint32_t i{}; i < code.size(); ++i)
    {
        if(code[i] == '.')
        {
            bf_instr_t instr{};
            instr.put = {BF_Opcode::PUT, cell_offset};
            code_.push_back(instr);
        }
        else if(code[i] == ',')
        {
            bf_instr_t instr{};
            instr.get = {BF_Opcode::GET, cell_offset};
            code_.push_back(instr);
        }
        else if(code[i] == '<')
        {
            --cell_offset;
        }
        else if(code[i] == '>')
        {
            ++cell_offset;
        }
        else if(code[i] == '+' || code[i] == '-')
        {
            std::uint8_t val(code[i] == '+' ? 1 : static_cast<std::uint8_t>(-1));

            if(!code_.empty() && code_.back().check.op == BF_Opcode::INC_CELL && code_.back().inc_cell.cell == cell_offset)
            {
                code_.back().inc_cell.val += val;
            }
            else
            {
                bf_instr_t instr{};
                instr.inc_cell = {BF_Opcode::INC_CELL, val, cell_offset};
                code_.push_back(instr);
            }
        }
        else if(code[i] == '[')
        {
            // Zero instruction
            if(code.size() > i + 2)
            {
                if(code[i + 1] == '-' || code[i + 1] == '+')
                {
                    if(code[i + 2] == ']')
                    {
                        bf_instr_t instr{};
                        instr.zero = {BF_Opcode::ZERO, cell_offset};
                        code_.push_back(instr);
                        i += 2;
                        continue;
                    }
                }
            }

            jmp_stack.push(code_.size());

            bf_instr_t instr{};
            instr.jmp_fwd = {BF_Opcode::JMP_FWD, cell_offset, 0};
            code_.push_back(instr);

            cell_offset = 0;
        }
        else if(code[i] == ']')
        {
            if(jmp_stack.empty())
            {
                std::clog << "Unmatched \']\'! Cleaned position: " << i << '\n';
                return false;
            }

            bf_instr_t instr{};
            instr.jmp_bwd = {BF_Opcode::JMP_BWD, cell_offset, jmp_stack.top() + 1};
            code_.push_back(instr);

            code_[jmp_stack.top()].jmp_fwd.dest = code_.size();
            jmp_stack.pop();
            cell_offset = 0;
        }
    }

    if(!jmp_stack.empty())
    {
        std::clog << "Unmatched \'[\'! Cleaned position: " << jmp_stack.top() << '\n';
        return false;
    }

    return true;
}

std::string BF_Interpreter::disassemble() const
{
    std::ostringstream strm;

    for(auto instr : code_)
    {
        switch(instr.check.op)
        {
        case BF_Opcode::INC_CELL:
            strm << "inc " << (int)instr.inc_cell.cell << ' ' << (int)instr.inc_cell.val << '\n';
            break;
        case BF_Opcode::JMP_FWD:
            strm << "jmp_fwd " << (int)instr.jmp_fwd.cell << ' ' << (int)instr.jmp_fwd.dest << '\n';
            break;
        case BF_Opcode::JMP_BWD:
            strm << "jmp_bwd " << (int)instr.jmp_bwd.cell << ' ' << (int)instr.jmp_bwd.dest << '\n';
            break;
        case BF_Opcode::ZERO:
            strm << "zero " << (int)instr.zero.cell << '\n';
            break;
        case BF_Opcode::PUT:
            strm << "put " << (int)instr.put.cell << '\n';
            break;
        case BF_Opcode::GET:
            strm << "get " << (int)instr.get.cell << '\n';
            break;
        case BF_Opcode::INVALID:
            strm << "invalid\n";
        }
    }

    return strm.str();
}
