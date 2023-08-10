#include "environment.h"

#include <cstddef>
#include <cassert>
#include <cstdlib>

#include "parser.h"

Environment *environmentCreate(Environment *parent){
    Environment *env = new Environment;
    assert(env && "Could not allocate memory for new environment.");
    env->parent = parent;
    env->bind = nullptr;
    return env;
}

int environmentSet(Environment *env, Node *id, Node *value){
    // Over-writing an existing value
    if (!env || !id || !value) {
        return 0;
    }
    Binding *binding_it = env->bind;
    while(binding_it){
        if(nodeCompare(binding_it->id, id)){
            binding_it->value = value;
            return 2;
        }
        binding_it = binding_it->next;
    }
    
    // Creating a new binding
    Binding *binding = new Binding;
    assert(binding && "Could not allocate new binding for the environment.");
    binding->id = id;
    binding->value = value;
    binding->next = env->bind;
    env->bind = binding;
    return 1;
}

bool environmentGet(Environment env, Node *id, Node *result){
    Binding *binding_it = env.bind;
    while(binding_it){
        if(nodeCompare(binding_it->id, id)){
            *result = *binding_it->value;
            return true;
        }
        binding_it = binding_it->next;
    }
    return false;
}

bool environmentGetBySymbol(Environment env, char *symbol, Node *result) {
    Node *symbol_node = nodeSymbol(symbol);
    bool status = environmentGet(env, symbol_node, result);
    delete symbol_node;
    return status;
}