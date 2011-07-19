/*
  Copyright (c) 2010-2011, Intel Corporation
  All rights reserved.

  Redistribution and use in source and binary forms, with or without
  modification, are permitted provided that the following conditions are
  met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.

    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.

    * Neither the name of Intel Corporation nor the names of its
      contributors may be used to endorse or promote products derived from
      this software without specific prior written permission.


   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS
   IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
   TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A
   PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER
   OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
   EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
   PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
   PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF
   LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
   NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
   SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  
*/

/** @file sym.h

    @brief header file with declarations for symbol and symbol table
    classes.
*/

#ifndef ISPC_SYM_H
#define ISPC_SYM_H

#include "ispc.h"
#include <map>

class StructType;
class ConstExpr;

/**
   @brief Representation of a program symbol.

   The Symbol class represents a symbol in an ispc program.  Symbols can
   include variables, functions, and named types.  Note that all of the
   members are publically accessible; other code throughout the system
   accesses and modifies the members directly.

   @todo Should we break function symbols into a separate FunctionSymbol
   class and then not have these members that are not applicable for
   function symbols (and vice versa, for non-function symbols)?
 */

class Symbol {
public:
    /** The Symbol constructor takes the name of the symbol, its
        position in a source file, and its type (if known). */
    Symbol(const std::string &name, SourcePos pos, const Type *t = NULL);

    /** This method should only be called for function symbols; for them,
        it returns a mangled version of the function name with the argument
        types encoded into the returned name.  This is used to generate
        unique symbols in object files for overloaded functions.
     */
    std::string MangledName() const;

    SourcePos pos;            /*!< Source file position where the symbol was defined */
    const std::string name;   /*!< Symbol's name */
    llvm::Value *storagePtr;  /*!< For symbols with storage associated with
                                   them (i.e. variables but not functions),
                                   this member stores a pointer to its
                                   location in memory.) */
    llvm::Function *function; /*!< For symbols that represent functions,
                                   this stores the LLVM Function value for
                                   the symbol once it has been created. */ 
    const Type *type;         /*!< The type of the symbol; if not set by the
                                   constructor, this is set after the
                                   declaration around the symbol has been parsed.  */
    ConstExpr *constValue;    /*!< For symbols with const-qualified types, this may store
                                   the symbol's compile-time constant value.  This value may
                                   validly be NULL for a const-qualified type, however; for
                                   example, the ConstExpr class can't currently represent
                                   struct types.  For cases like these, ConstExpr is NULL,
                                   though for all const symbols, the value pointed to by the
                                   storagePtr member will be its constant value.  (This
                                   messiness is due to needing an ispc ConstExpr for the early 
                                   constant folding optimizations). */
    bool isStatic;            /*!< Records whether this symbol had a static qualifier in
                                   its declaration. */
    int varyingCFDepth;       /*!< This member records the number of levels of nested 'varying' 
                                   control flow within which the symbol was declared.  Having
                                   this value available makes it possible to avoid performing
                                   masked stores when modifying the symbol's value when the
                                   store is done at the same 'varying' control flow depth as 
                                   the one where the symbol was originally declared. */
};


/** @brief Symbol table that holds all known symbols during parsing and compilation.

    A single instance of a SymbolTable is stored in the Module class
    (Module::symbolTable); it is created in the Module::Module()
    constructor.  It is then accessed via the global variable Module *\ref m
    throughout the ispc implementation.
 */

class SymbolTable {
public:
    SymbolTable();
    ~SymbolTable();

    /** The parser calls this method when it enters a new scope in the
        program; this allows us to track variables that shadows others in
        outer scopes with same name as well as to efficiently discard all
        of the variables declared in a particular scope when we exit that
        scope. */
    void PushScope();

    /** For each scope started by a call to SymbolTable::PushScope(), there
        must be a matching call to SymbolTable::PopScope() at the end of
        that scope. */ 
    void PopScope();

    /** Adds the given variable symbol to the symbol table.
        @param symbol The symbol to be added

        @return true if successful; false if the provided symbol clashes
        with a symbol defined at the same scope.  (Symbols may shaodow
        symbols in outer scopes; a warning is issued in this case, but this
        method still returns true.) */
    bool AddVariable(Symbol *symbol);

    /** Looks for a variable with the given name in the symbol table.  This
        method searches outward from the innermost scope to the outermost,
        returning the first match found.

        @param  name The name of the variable to be searched for.
        @return A pointer to the Symbol, if a match is found.  NULL if no 
        Symbol with the given name is in the symbol table. */
    Symbol *LookupVariable(const char *name);

    /** Adds the given function symbol to the symbol table.
        @param symbol The function symbol to be added.

        @return true if the symbol has been added.  False if another
        function symbol with the same name and function signature is
        already present in the symbol table. */
    bool AddFunction(Symbol *symbol);

    /** Looks for the function or functions with the given name in the
        symbol name.  If a function has been overloaded and multiple
        definitions are present for a given function name, all of them will
        be returned and it's up the the caller to resolve which one (if
        any) to use.

        @return vector of Symbol pointers to functions with the given name. */
    std::vector<Symbol *> *LookupFunction(const char *name);

    /** Looks for a function with the given name and type
        in the symbol table.

        @return pointer to matching Symbol; NULL if none is found. */
    Symbol *LookupFunction(const char *name, const FunctionType *type);

    /** Returns all of the functions in the symbol table that match the given 
        predicate.

        @param pred A unary predicate that returns true or false, given a Symbol 
        pointer, based on whether the symbol should be included in the returned 
        set of matches.  It can either be a function, with signature 
        <tt>bool pred(const Symbol *s)</tt>, or a unary predicate object with 
        an <tt>bool operator()(const Symbol *)</tt> method.

        @param matches Pointer to a vector in which to return the matching
        symbols. 
     */
    template <typename Predicate> 
        void GetMatchingFunctions(Predicate pred, 
                                  std::vector<Symbol *> *matches) const;

    /** Adds the named type to the symbol table.  This is used for both
        struct definitions (where <tt>struct Foo</tt> causes type \c Foo to
        be added to the symbol table) as well as for <tt>typedef</tt>s.

        @param name Name of the type to be added
        @param type Type that \c name represents
        @param pos Position in source file where the type was named
        @return true if the named type was successfully added.  False if a type
        with the same name has already been defined.
        
    */
    bool AddType(const char *name, const Type *type, SourcePos pos);

    /** Looks for a type of the given name in the symbol table.

        @return Pointer to the Type, if found; otherwise NULL is returned.
    */
    const Type *LookupType(const char *name) const;

    /** This method returns zero or more strings with the names of symbols
        in the symbol table that nearly (but not exactly) match the given
        name.  This is useful for issuing informative error methods when
        misspelled identifiers are found a programs.

        @param name String to compare variable and function symbol names against.
        @return vector of zero or more strings that approximately match \c name.
    */
    std::vector<std::string> ClosestVariableOrFunctionMatch(const char *name) const;
    /** This method returns zero or more strings with the names of types
        in the symbol table that nearly (but not exactly) match the given
        name. */
    std::vector<std::string> ClosestTypeMatch(const char *name) const;

    std::vector<std::string> ClosestEnumTypeMatch(const char *name) const;

    /** Prints out the entire contents of the symbol table to standard error.
        (Debugging method). */
    void Print();

private:
    std::vector<std::string> closestTypeMatch(const char *str, 
                                              bool structsVsEnums) const;

    /** This member variable holds one \c vector of Symbol pointers for
        each of the current active scopes as the program is being parsed.
        New vectors of symbols are added and removed from the end of the
        main vector, so searches for symbols start looking at the end of \c
        variables and work backwards.
     */
    std::vector<std::vector<Symbol *> *> variables;
    /** Because there is no scoping for function symbols, functions are
        represented with a single STL \c map from names to symbols.  A STL
        \c vector is used to store the function symbols for a given name
        since, due to function overloading, a name can have multiple
        function symbols associated with it. */
    std::map<std::string, std::vector<Symbol *> > functions;
    typedef std::map<std::string, const Type *> TypeMapType;
    /** Like variables, type definitions can be scoped.  A new \c TypeMapType
        is added to the back of the \c types \c vector each time a new scope
        is entered.  (And it's removed when the scope exits).
     */
    std::vector<TypeMapType *> types;
};


template <typename Predicate> 
void SymbolTable::GetMatchingFunctions(Predicate pred, 
                                       std::vector<Symbol *> *matches) const {
    // Iterate through all function symbols and apply the given predicate.
    // If it returns true, add the Symbol * to the provided vector.
    std::map<std::string, std::vector<Symbol *> >::const_iterator iter;
    for (iter = functions.begin(); iter != functions.end(); ++iter) {
        const std::vector<Symbol *> &syms = iter->second;
        for (unsigned int i = 0; i < syms.size(); ++i) {
            if (pred(syms[i]))
                matches->push_back(syms[i]);
        }
    }
}

#endif // ISPC_SYM_H
