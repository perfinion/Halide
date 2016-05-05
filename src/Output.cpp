#include "CodeGen_C.h"
#include "StmtToHtml.h"
#include "Output.h"
#include "LLVM_Headers.h"
#include "LLVM_Output.h"
#include "LLVM_Runtime_Linker.h"

#include <fstream>

namespace Halide {

void compile_module_to(const Module &module, const Outputs &output_files) {
    llvm::LLVMContext context;
    std::unique_ptr<llvm::Module> llvm_module(compile_module_to_llvm_module(module, context));

    if (!output_files.object_name.empty()) {
        if (module.target().arch == Target::PNaCl) {
            compile_llvm_module_to_llvm_bitcode(*llvm_module, output_files.object_name);
        } else {
            compile_llvm_module_to_object(*llvm_module, output_files.object_name);
        }
    }
    if (!output_files.assembly_name.empty()) {
        if (module.target().arch == Target::PNaCl) {
            compile_llvm_module_to_llvm_assembly(*llvm_module, output_files.assembly_name);
        } else {
            compile_llvm_module_to_assembly(*llvm_module, output_files.assembly_name);
        }
    }
    if (!output_files.bitcode_name.empty()) {
        compile_llvm_module_to_llvm_bitcode(*llvm_module, output_files.bitcode_name);
    }
    if (!output_files.llvm_assembly_name.empty()) {
        compile_llvm_module_to_llvm_assembly(*llvm_module, output_files.llvm_assembly_name);
    }
}

void compile_module_to_object(const Module &module, std::string filename) {
    if (filename.empty()) {
        if (module.target().os == Target::Windows &&
            !module.target().has_feature(Target::MinGW)) {
            filename = module.name() + ".obj";
        } else {
            filename = module.name() + ".o";
        }
    }

    compile_module_to(module, Outputs().object(filename));
}

void compile_module_to_assembly(const Module &module, std::string filename)  {
    if (filename.empty()) filename = module.name() + ".s";

    compile_module_to(module, Outputs().assembly(filename));
}

void compile_module_to_native(const Module &module,
                   std::string object_filename,
                   std::string assembly_filename) {
    if (object_filename.empty()) {
        if (module.target().os == Target::Windows &&
            !module.target().has_feature(Target::MinGW)) {
            object_filename = module.name() + ".obj";
        } else {
            object_filename = module.name() + ".o";
        }
    }
    if (assembly_filename.empty()) {
        assembly_filename = module.name() + ".s";
    }

    compile_module_to(module, Outputs().object(object_filename).assembly(assembly_filename));
}

void compile_module_to_llvm_bitcode(const Module &module, std::string filename)  {
    if (filename.empty()) filename = module.name() + ".bc";

    compile_module_to(module, Outputs().bitcode(filename));
}

void compile_module_to_llvm_assembly(const Module &module, std::string filename)  {
    if (filename.empty()) filename = module.name() + ".ll";

    compile_module_to(module, Outputs().llvm_assembly(filename));
}

void compile_module_to_llvm(const Module &module,
                 std::string bitcode_filename,
                 std::string llvm_assembly_filename)  {
    if (bitcode_filename.empty()) bitcode_filename = module.name() + ".bc";
    if (llvm_assembly_filename.empty()) llvm_assembly_filename = module.name() + ".ll";

    compile_module_to(module, Outputs().bitcode(bitcode_filename).llvm_assembly(llvm_assembly_filename));
}

void compile_module_to_html(const Module &module, std::string filename) {
    if (filename.empty()) filename = module.name() + ".html";

    Internal::print_to_html(filename, module);
}

void compile_module_to_text(const Module &module, std::string filename) {
    if (filename.empty()) filename = module.name() + ".stmt";

    std::ofstream file(filename.c_str());
    file << module;
}

void compile_module_to_c_header(const Module &module, std::string filename) {
    if (filename.empty()) filename = module.name() + ".h";

    std::ofstream file(filename.c_str());
    Internal::CodeGen_C cg(file,
                           module.target().has_feature(Target::CPlusPlusMangling) ?
                           Internal::CodeGen_C::CPlusPlusHeader : Internal::CodeGen_C::CHeader,
                           filename);
    cg.compile(module);
}

void compile_module_to_c_source(const Module &module, std::string filename) {
    if (filename.empty()) filename = module.name() + ".c";

    std::ofstream file(filename.c_str());
    Internal::CodeGen_C cg(file,
                           module.target().has_feature(Target::CPlusPlusMangling) ?
                           Internal::CodeGen_C::CPlusPlusImplementation : Internal::CodeGen_C::CImplementation);
    cg.compile(module);
}

void compile_module_to_c(const Module &module,
              std::string h_filename,
              std::string c_filename) {
    compile_module_to_c_header(module, h_filename);
    compile_module_to_c_source(module, c_filename);
}

void compile_standalone_runtime(std::string object_filename, Target t) {
    t.set_feature(Target::NoRuntime, false);
    t.set_feature(Target::JIT, false);
    Module empty("standalone_runtime", t);
    compile_module_to_object(empty, object_filename);
}

}  // namespace Halide
