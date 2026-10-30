#pragma once

#include <vector>

#include "middleend/transforms/trivial_dce.hpp"
#include "middleend/transforms/dse.hpp"
#include "middleend/transforms/cleanup_cfg.hpp"
#include "middleend/transforms/mem2reg.hpp"
#include "middleend/transforms/cse.hpp"

#include "utils/fatal.hpp"

using PassCallback = void(*)(IRProgram&);

enum class Transforms
{
    INLINE,
    SROA,
    TRIVIAL_DCE,
    DSE,
    CONST_FOLD,
    CFG_CLEANUP,
    MEM2REG,
    CSE,
    LICM,
};

class TransformPassManager
{
public:
    TransformPassManager(std::vector<Transforms> transforms)
    {
        for (auto t : transforms) {
            switch (t) {
                case Transforms::TRIVIAL_DCE:
                    add_pass(trivial_dce);
                    break;

                case Transforms::DSE:
                    add_pass(remove_dead_stores);
                    break;

                case Transforms::CFG_CLEANUP:
                    add_pass(cleanup_cfg);
                    break;

                case Transforms::MEM2REG:
                    add_pass(mem2reg);
                    break;

                case Transforms::CSE:
                    add_pass(cse);
                    break;

                default:
                    fatal_error("transform pass not supported");
            }
        }
    }

    void run(IRProgram& program)
    {
        for (auto& pass : passes_) {
            pass(program);
        }
    }

private:
    std::vector<PassCallback> passes_;

    void add_pass(PassCallback pass)
    {
        passes_.push_back(pass);
    }
};
