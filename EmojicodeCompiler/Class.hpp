//
//  Class.hpp
//  Emojicode
//
//  Created by Theo Weidmann on 09/01/16.
//  Copyright © 2016 Theo Weidmann. All rights reserved.
//

#ifndef Class_hpp
#define Class_hpp

#include <list>
#include <vector>
#include <map>
#include <set>
#include "Package.hpp"

class TypeDefinition {
public:
    /** Returns a documentation token documenting this type definition or @c nullptr. */
    const Token* documentationToken() const { return documentationToken_; }
    /** Returns the name of the type definition. */
    EmojicodeChar name() const { return name_; }
    /** Returns the package in which this type was defined. */
    Package* package() const { return package_; }
protected:
    TypeDefinition(EmojicodeChar name, Package *p, const Token *dToken)
        : name_(name), package_(p), documentationToken_(dToken) {}
private:
    const Token *documentationToken_;
    EmojicodeChar name_;
    Package *package_;
};

class TypeDefinitionWithGenerics : public TypeDefinition {
public:
    void addGenericArgument(const Token *variable, Type constraint);
    void setSuperTypeDef(TypeDefinitionWithGenerics *superTypeDef);
    void setSuperGenericArguments(std::vector<Type> superGenericArguments);
    void finalizeGenericArguments();
    
    /** Returns the number of generic arguments this type takes when referenced to in Emojicode source code. */
    uint16_t numberOfOwnGenericArguments() const { return ownGenericArgumentCount_; }
    /** 
     * Returns the number of generic arguments a type of this type definition stores when initialized.
     * This therefore also includes all arguments to supertypedefinitions of this type.
     */
    uint16_t numberOfGenericArgumentsWithSuperArguments() const { return genericArgumentCount_; }
    /** 
     * Tries to fetch the type reference type for the given generic variable name and stores it into @c type.
     * @returns Whether the variable could be found or not. @c type is untouched if @c false was returned.
     */
    bool fetchVariable(EmojicodeString name, bool optional, Type *type);
    /*
     * Determines whether the given type reference resolution constraint allows the type to be
     * resolved on this type definition.
     */
    virtual bool canBeUsedToResolve(TypeDefinitionWithGenerics *resolutionConstraint) = 0;
    
    const std::map<EmojicodeString, Type>& ownGenericArgumentVariables() const { return ownGenericArgumentVariables_; }
    const std::vector<Type>& superGenericArguments() const { return superGenericArguments_; }
    const std::vector<Type>& genericArgumentConstraints() const { return genericArgumentConstraints_; }
protected:
    TypeDefinitionWithGenerics(EmojicodeChar name, Package *p, const Token *dToken) : TypeDefinition(name, p, dToken) {}
private:
    /** The number of generic arguments including those from a super eclass. */
    uint16_t genericArgumentCount_ = 0;
    /** The number of generic arguments this eclass takes. */
    uint16_t ownGenericArgumentCount_ = 0;
    /** The types for the generic arguments. */
    std::vector<Type> genericArgumentConstraints_;
    /** The arguments for the classes from which this eclass inherits. */
    std::vector<Type> superGenericArguments_;
    /** Generic type arguments as variables */
    std::map<EmojicodeString, Type> ownGenericArgumentVariables_;
};

class Class : public TypeDefinitionWithGenerics {
public:
    static const std::list<Class *>& classes() { return classes_; }
    
    Class(EmojicodeChar name, const Token *classBegin, Package *pkg, const Token *dToken);
    
    /** Whether this eclass eligible for initializer inheritance. */
    bool inheritsContructors = false;
    
    /** The class's superclass. @c nullptr if the class has no superclass. */
    Class *superclass = nullptr;
    
    uint16_t index;
    
    const Token* classBeginToken() const { return classBeginToken_; }
    
    bool canBeUsedToResolve(TypeDefinitionWithGenerics *a);
    
    /** The variable names. */
    std::vector<Variable *> instanceVariables;
    
    /** List of all methods for user classes */
    std::vector<Method *> methodList;
    std::vector<Initializer *> initializerList;
    std::vector<ClassMethod *> classMethodList;
    const std::set<EmojicodeChar>& requiredInitializers() const { return requiredInitializers_; }
    
    uint16_t nextMethodVti;
    uint16_t nextClassMethodVti;
    uint16_t nextInitializerVti;

    /** Returns true if @c a inherits from eclass @c from */
    bool inheritsFrom(Class *from);
    
    /** Returns a method by the given identifier token or issues an error if the method does not exist. */
    Method* getMethod(const Token *token, Type type, TypeContext typeContext);
    /** Returns an initializer by the given identifier token or issues an error if the initializer does not exist. */
    Initializer* getInitializer(const Token *token, Type type, TypeContext typeContext);
    /** Returns a method by the given identifier token or issues an error if the method does not exist. */
    ClassMethod* getClassMethod(const Token *token, Type type, TypeContext typeContext);
    
    /** Returns a method by the given identifier token or @c nullptr if the method does not exist. */
    Method* lookupMethod(EmojicodeChar name);
    /** Returns a initializer by the given identifier token or @c nullptr if the initializer does not exist. */
    Initializer* lookupInitializer(EmojicodeChar name);
    /** Returns a method by the given identifier token or @c nullptr if the method does not exist. */
    ClassMethod* lookupClassMethod(EmojicodeChar name);
    
    void addMethod(Method *method);
    void addInitializer(Initializer *method);
    void addClassMethod(ClassMethod *method);
    
    /** Declares that this class agrees to the given protocol. */
    bool addProtocol(Type type);
    /** Returns a list of all protocols to which this class conforms. */
    const std::list<Type>& protocols() const { return protocols_; };
private:
    static std::list<Class *> classes_;
    
    std::map<EmojicodeChar, Method *> methods_;
    std::map<EmojicodeChar, ClassMethod *> classMethods_;
    std::map<EmojicodeChar, Initializer *> initializers_;
    
    std::list<Type> protocols_;
    std::set<EmojicodeChar> requiredInitializers_;
    
    const Token *classBeginToken_;
};

class Protocol : public TypeDefinitionWithGenerics {
public:
    Protocol(EmojicodeChar name, Package *pkg, const Token *dt);
    
    uint_fast16_t index;
    
    bool canBeUsedToResolve(TypeDefinitionWithGenerics *a);
    
    Method* getMethod(const Token *token, Type type, TypeContext typeContext);
    Method* lookupMethod(EmojicodeChar name);
    void addMethod(Method *method);
    const std::vector<Method*>& methods() { return methodList_; };
    
    bool usesSelf() const { return usesSelf_; }
    void setUsesSelf() { usesSelf_ = true; }
private:
    static uint_fast16_t nextIndex;
    
    /** List of all methods. */
    std::vector<Method *> methodList_;
    
    bool usesSelf_ = false;
    
    /** Hashmap holding methods. @warning Don't access it directly, use the correct functions. */
    std::map<EmojicodeChar, Method*> methods_;
};

class Enum : public TypeDefinition {
public:
    Enum(EmojicodeChar name, Package *package, const Token *dt)
        : TypeDefinition(name, package, dt) {}
    
    std::pair<bool, EmojicodeInteger> getValueFor(EmojicodeChar c) const;
    void addValueFor(EmojicodeChar c);
    
    const std::map<EmojicodeChar, EmojicodeInteger>& values() const { return map_; }
private:
    std::map<EmojicodeChar, EmojicodeInteger> map_;
    EmojicodeInteger valuesCounter = 0;
};

#endif /* Class_hpp */
