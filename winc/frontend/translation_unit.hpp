#pragma once

#include "utils/mapped_file.hpp"
#include "frontend/error/diagnostics.hpp"
#include "frontend/sema/types/type_table.hpp"
#include "frontend/sema/symbol.hpp"

struct TranslationUnit
{
    TranslationUnit(std::string_view file_path) :
        name_{ file_path },
        source_file_{ file_path },
        type_table_{ },
        symbol_table_{ },
        diagnostics_{ } {}

    const std::string name_;
    MappedFile source_file_;
    TypeTable type_table_;
    SymbolTable symbol_table_;
    Diagnostics diagnostics_;
};
