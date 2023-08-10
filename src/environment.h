#ifndef COMPILER_ENVIRONMENT_H
#define COMPILER_ENVIRONMENT_H

typedef struct Node Node;

struct Binding {
    Node *id;
    Node *value;
    Binding *next;
};

struct Environment {
    Environment *parent;
    Binding *bind;
};

Environment *environmentCreate(Environment *parent);
/**
 * @retval 0 Failure.
 * @retval 1 Creation of new binding.
 * @retval 2 Existing binding value overwrite (ID unused).
 */
int environmentSet(Environment *env, Node *id, Node *value);
bool environmentGet(Environment env, Node *id, Node *result);
bool environmentGetBySymbol(Environment env, char *symbol, Node *result);

#endif /* COMPILER_ENVIRONMENT_H */