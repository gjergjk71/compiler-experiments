#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

const int ROOT = 1;
const int IDENTIFIER = 2;
const int DECLARATION = 3;
const int ASSIGNMENT = 4;
const int STRING = 111;
const int INTEGER = 222;
const int FLOAT = 333;
const int SYMBOL = 8;
const int OPERATOR = 9;
const int SEMICOLON = 10;
const int CONDITIONAL = 11;
const int END = 12;
const int COLON = 13;
const int COMMENT = 14;
const int OPEN_PARENTHESIS = 15;
const int CLOSE_PARENTHESIS = 16;
const int COMMA = 17;
const int TYPE = 18;

const int AST_NODE_CONSTANT = 30;
const int AST_NODE_ADD = 31;
const int AST_NODE_SUB = 32;
const int AST_NODE_MULTIPLE = 33;
const int AST_NODE_DIVIDE = 34;
const int AST_NODE_DECLARATION = 35;
const int AST_NODE_IDENTIFIER = 36;
const int AST_NODE_OPERATOR = 37;
const int AST_NODE_IDENTIFIER_INIT = 38;

struct token
{
    int type;
    char *value;
    struct token *next;
};
// ["var","name","=","Gjergj",";"]

struct ast_node
{
    int type;
    struct token *token;
    struct ast_node *left;
    struct ast_node *right;
};

struct ast_nodes
{
    struct ast_node *current;
    struct ast_nodes *next;
};

struct symbol_node
{
    int type;
    char *name;
    char *value;
    struct symbol_node *next;
};

struct token *addTokens(int type, char value[], struct token *root_token);
struct token *addTokens2(int type, char value[], struct token *root_token);
struct token *addTokens3(struct token *root_token);
struct token *addTokens4(struct token *root_token);
int token_is_type(struct token *token, int type);
int rule0(struct token *token, struct ast_node **ast_node);
int rule5(struct token *token, struct ast_node **ast_node);
int rule1_2_3(struct token *token, struct ast_node **ast_node, int type);
int rule4(struct token *token, struct ast_node **ast_node, struct token **cToken);
int get_ast(struct token *root_token, struct ast_nodes **ast_nodes_pp);
void show_ast_node(struct ast_node *ast_node);
void show_ast_nodes(struct ast_nodes *ast_nodes);
void lex(char *sourceCode, struct token *root_token);
void slice_str(const char *str, char *buffer, size_t start, size_t end);
void *evaluateNode(struct ast_node *ast_node, struct symbol_node *root_symbol_node);
void evaluate(struct ast_nodes *ast_nodes, struct symbol_node *root_symbol_node);
void add_symbol(struct symbol_node *root_symbol_node, struct symbol_node *new_symbol);

void show_tokens(struct token *root_token)
{
    struct token *cToken = root_token;
    while (cToken)
    {
        printf("Token: %s - %d\n", cToken->value, cToken->type);
        cToken = cToken->next;
    }
    printf("show_tokens finished\n");
}

int main(int argc, char *argv[])
{
    struct token *root_token = malloc(sizeof(struct token));
    root_token->type = ROOT;
    struct ast_nodes *ast_nodes = malloc(sizeof(struct ast_nodes));
    // ast_nodes->current->type = -1;
    struct symbol_node *root_symbol_node = malloc(sizeof(struct symbol_node));
    if (argc > 1)
    {
        char *buffer = 0;
        long length;
        FILE *f = fopen(argv[1], "rb");

        if (f)
        {
            fseek(f, 0, SEEK_END);
            length = ftell(f);
            fseek(f, 0, SEEK_SET);
            buffer = malloc(length);
            if (buffer)
            {
                fread(buffer, 1, length, f);
            }
            fclose(f);
        }
        else
        {
            printf("File %s not found.\n", argv[1]);
            return 1;
        }

        if (buffer)
        {
            //printf("%s\n",buffer);
            lex(buffer, root_token);
            show_tokens(root_token);
            get_ast(root_token, &ast_nodes);
            show_ast_nodes(ast_nodes);
            evaluate(ast_nodes, root_symbol_node);
        }
    }
    return 0;
}

void *evaluateNode(struct ast_node *ast_node, struct symbol_node *root_symbol_node)
{
    if (ast_node->type == AST_NODE_CONSTANT)
    {
        // printf("CONSTANT %s\n", ast_node->token->value);
        return ast_node->token->value;
    }
    else if (ast_node->type == AST_NODE_OPERATOR)
    {
        char *left = evaluateNode(ast_node->left, root_symbol_node);
        char *right = evaluateNode(ast_node->right, root_symbol_node);
        float value = -999999;
        if (ast_node->token->value[0] == '+')
            value = atof(left) + atof(right);
        if (ast_node->token->value[0] == '-')
            value = atof(left) - atof(right);
        if (ast_node->token->value[0] == '*')
            value = atof(left) * atof(right);
        if (ast_node->token->value[0] == '/')
            value = atof(left) / atof(right);
        char *str_value = malloc(sizeof(char) * 500);
        snprintf(str_value, sizeof(str_value), "%2.5f", value);
        return str_value;
    }
    else if (ast_node->type == AST_NODE_DECLARATION)
    {
        char *left = ast_node->left->token->value; //evaluateNode(ast_node->left,root_symbol_node);
        char *right = evaluateNode(ast_node->right, root_symbol_node);
        struct symbol_node *symbol_node = malloc(sizeof(struct symbol_node));
        symbol_node->name = left;
        symbol_node->type = ast_node->right->token->type;
        symbol_node->value = right;
        add_symbol(root_symbol_node, symbol_node);
        return right;
    }
    else if (ast_node->type == AST_NODE_IDENTIFIER_INIT)
    {
        printf("HERE5555\n");
        return ast_node->token->value;
    }
    else if (ast_node->type == AST_NODE_IDENTIFIER)
    {
        struct symbol_node *c_symbol_node = root_symbol_node->next;
        while (c_symbol_node != NULL)
        {
            if (strcmp(c_symbol_node->name, ast_node->token->value) == 0)
            {
                return c_symbol_node->value;
            }
            c_symbol_node = c_symbol_node->next;
        }
        printf("NOT FOUND\n");
    }
    printf("[%d] FAILED;\n", ast_node->type);
}

void add_symbol(struct symbol_node *root_symbol_node, struct symbol_node *new_symbol)
{
    while (root_symbol_node->next != NULL)
        root_symbol_node = root_symbol_node->next;
    root_symbol_node->next = new_symbol;
}

void evaluate(struct ast_nodes *ast_nodes, struct symbol_node *root_symbol_node)
{
    while (ast_nodes != NULL && ast_nodes->current->type != 0)
    {
        struct ast_node *c_ast_node = ast_nodes->current;
        char *value = evaluateNode(c_ast_node, root_symbol_node);
        printf("[%d] Returned: %s\n", c_ast_node->type, value);
        ast_nodes = ast_nodes->next;
    }
}

void lex(char *sourceCode, struct token *root_token)
{
    struct token *cToken = root_token;
    size_t sz = strlen(sourceCode);
    for (int x = 0; x < sz; x++)
    {
        printf("x: %d\n", x);
        while (isspace(sourceCode[x]))
            x++;
        int startIndex = -1;
        int endIndex = -1;
        struct token *nxt_token = malloc(sizeof(struct token));
        nxt_token->type = -1;
        nxt_token->value = NULL;
        nxt_token->next = NULL;
        char *special_value = malloc(sizeof(char) * 3);
        special_value[2] = '\0';
        if (sourceCode[x] == ';')
        {
            nxt_token->type = SEMICOLON;
            special_value[0] = ';';
            nxt_token->value = special_value;
        }
        else if (sourceCode[x] == '(')
        {
            nxt_token->type = OPEN_PARENTHESIS;
            special_value[0] = '(';
            nxt_token->value = special_value;
        }
        else if (sourceCode[x] == ')')
        {
            nxt_token->type = CLOSE_PARENTHESIS;
            special_value[0] = ')';
            nxt_token->value = special_value;
        }
        else if (sourceCode[x] == '=')
        {
            nxt_token->type = ASSIGNMENT;
            special_value[0] = '=';
            nxt_token->value = special_value;
        }
        else if (
            sourceCode[x] == '+' || sourceCode[x] == '-' ||
            sourceCode[x] == '*' || sourceCode[x] == '/')
        {
            nxt_token->type = OPERATOR;
            special_value[0] = sourceCode[x];
            nxt_token->value = special_value;
            printf("special_value %s\n", special_value);
        }
        else if (sourceCode[x] == '"')
        {
            printf("OPEN_APOS\n");
            x++;
            int startI = x;
            while (sourceCode[x] != '"')
                x++;
            char *value = malloc(sizeof(sizeof(char) * (x - startI)));
            slice_str(sourceCode, value, startI, x - 1);
            nxt_token->type = STRING;
            nxt_token->value = value;
            printf("OPEN_APOS: %s\n", value);
        }
        else
        {
            startIndex = x;
            while (!isspace(sourceCode[x + 1]))
                x++;
            endIndex = x;

            char *value = malloc(sizeof(sizeof(char) * (endIndex - startIndex)));
            slice_str(sourceCode, value, startIndex, endIndex);
            printf("%s\n", value);
            if (strcmp(value, "var") == 0)
            {
                nxt_token->type = DECLARATION;
                nxt_token->value = value;
            }
            else if (isalpha(sourceCode[startIndex]))
            {
                nxt_token->type = IDENTIFIER;
                nxt_token->value = value;
            }
            else if (isdigit(sourceCode[startIndex]))
            {
                nxt_token->type = INTEGER;
                nxt_token->value = value;
            }
        }
        if (nxt_token->type != -1)
        {
            cToken->next = nxt_token;
            cToken = cToken->next;
        }
    }
    printf("FINISHED\n");
}

int token_is_type(struct token *token, int type)
{
    if (token != NULL)
    {
        if (token->type == type)
            return 1;
    }
    // printf("Not the same --> %s -- %d %d\n",token->value,type,token->type);
    return 0;
}

int rule0(struct token *token, struct ast_node **ast_node)
{
    if (
        token_is_type(token, STRING) ||
        token_is_type(token, INTEGER) ||
        token_is_type(token, FLOAT))
    {
        (*ast_node)->type = AST_NODE_CONSTANT;
        (*ast_node)->token = token;
        return 1;
    }
    return 0;
}

int rule5(struct token *token, struct ast_node **ast_node)
{
    if (
        token_is_type(token, IDENTIFIER))
    {
        (*ast_node)->type = AST_NODE_IDENTIFIER;
        (*ast_node)->token = token;
        return 1;
    }
    return 0;
}

int rule1_2_3(struct token *token, struct ast_node **ast_node, int type)
{
    if (
        token_is_type(token, DECLARATION) &&
        token_is_type(token->next, IDENTIFIER) &&
        token_is_type(token->next->next, ASSIGNMENT) &&
        token_is_type(token->next->next->next, type))
    {
        struct ast_node *ast_node_left = malloc(sizeof(struct ast_node));
        struct ast_node *ast_node_right = malloc(sizeof(struct ast_node));

        rule5(token->next, &ast_node_left);
        rule0(token->next->next->next, &ast_node_right);

        (*ast_node)->type = AST_NODE_DECLARATION;
        (*ast_node)->left = ast_node_left;
        (*ast_node)->right = ast_node_right;
        return 1;
    }
    return 0;
}

int rule4(struct token *token, struct ast_node **ast_node, struct token **cToken)
{ // cToken internal
    // EBNF:
    // op = "+" | "-" | "*" | "/"
    // number = "1" | "2" ...
    // expression = (op expression expression)

    if (cToken != NULL)
        token = *cToken;

    int rule0_valid = rule0(token, ast_node);
    if (rule0_valid)
    {
        *cToken = token->next;
        return 1;
    };
    int rule5_valid = rule5(token, ast_node);
    if (rule5_valid)
    {
        *cToken = token->next;
        return 1;
    };
    if (
        token_is_type(token, OPEN_PARENTHESIS) &&
        token_is_type(token->next, OPERATOR))
    {
        struct token *op_token = token->next;
        struct ast_node *ast_node_left = malloc(sizeof(struct ast_node));
        struct ast_node *ast_node_right = malloc(sizeof(struct ast_node));

        token = token->next->next;
        int inside_r4_1 = rule4(token, &ast_node_left, &token);
        int inside_r4_2 = rule4(token, &ast_node_right, &token);

        if (inside_r4_1 && inside_r4_2 && token->type == CLOSE_PARENTHESIS)
        {
            *cToken = token;
            if (token->next)
                *cToken = token->next;
            (*ast_node)->type = AST_NODE_OPERATOR;
            (*ast_node)->token = op_token;
            printf("(*ast_node)->token %d %s\n", op_token->type, op_token->value);
            (*ast_node)->left = ast_node_left;
            (*ast_node)->right = ast_node_right;
            return 1;
        }
        else
        {
        }
    }
    return 0;
}

int rule6(struct token *token, struct ast_node **ast_node, struct token **cToken)
{
    if (
        token_is_type(token, DECLARATION) &&
        token_is_type(token->next, IDENTIFIER) &&
        token_is_type(token->next->next, ASSIGNMENT) &&
        token_is_type(token->next->next->next, OPEN_PARENTHESIS))
    {
        struct ast_node *ast_node_left = malloc(sizeof(struct ast_node));
        struct ast_node *ast_node_right = malloc(sizeof(struct ast_node));

        rule5(token->next, &ast_node_left);

        int rule4_valid = rule4(token->next->next->next, &ast_node_right, &token->next->next->next);
        if (!rule4_valid)
        {
            printf("---> !rule4_valid\n");
            return 0;
        }
        *cToken = token->next->next->next;

        (*ast_node)->type = AST_NODE_DECLARATION;
        (*ast_node)->left = ast_node_left;
        (*ast_node)->right = ast_node_right;
        return 1;
    }
    return 0;
}

int get_ast(struct token *root_token, struct ast_nodes **ast_nodes_pp)
{
    struct token *cToken = root_token->next;
    struct ast_nodes *ast_nodes = *ast_nodes_pp;

    while (1 && cToken != NULL)
    {
        // printf("%d=%s\n", cToken->type, cToken->value);
        cToken = cToken->next;
    }
    // printf("DEBUG END\n\n");
    cToken = root_token->next;
    while (cToken != NULL)
    {
        ast_nodes->current = malloc(sizeof(struct ast_node));
        ast_nodes->current->type = -1;
        struct ast_node **ast_node = &ast_nodes->current;

        int rule0_valid = rule0(cToken, ast_node);
        int rule1_valid = rule1_2_3(cToken, ast_node, STRING);
        int rule2_valid = rule1_2_3(cToken, ast_node, INTEGER);
        int rule3_valid = rule1_2_3(cToken, ast_node, FLOAT);
        int rule5_valid = rule5(cToken, ast_node);
        struct token *rule4_toToken = cToken;
        int rule4_valid = rule4(cToken, ast_node, &rule4_toToken);

        struct token *rule6_toToken = cToken;
        int rule6_valid = rule6(cToken, ast_node, &rule6_toToken);

        // printf("rules: %d %d %d %d\n", rule0_valid, rule1_valid, rule2_valid, rule3_valid);
        // printf("cToken type: %d\n", cToken->type);
        if (rule1_valid || rule2_valid || rule3_valid)
        {
            ast_nodes->current->left->type = AST_NODE_IDENTIFIER_INIT;
        }
        if (rule1_valid)
        {
            cToken = cToken->next->next->next->next;
            printf("Rule 1 semi-passed.\n");
        }
        else if (rule2_valid)
        {
            cToken = cToken->next->next->next->next;
            printf("Rule 2 semi-passed.\n");
        }
        else if (rule3_valid)
        {
            cToken = cToken->next->next->next->next;
            printf("Rule 3 semi-passed.\n");
        }
        else if (rule4_valid)
        {
            cToken = rule4_toToken;
            printf("Rule 4 semi-passed. [%p]\n", cToken);
            printf("Rule 4 semi-passed. type: %d val: %s\n", cToken->type, cToken->value);
        }
        else if (rule5_valid)
        {
            printf("Rule 5 semi-passed.");
            cToken = cToken->next;
        }
        else if (rule0_valid)
        {
            cToken = cToken->next;
            printf("Rule 0 semi-passed.\n");
        }
        else if (rule6_valid)
        {
            cToken = rule6_toToken;
            printf("Rule 6 semi-passed;\n");
        }
        else
        {
            printf("No rule valids %d - %s\n", cToken->type, cToken->value);
            return -1;
        }
        // printf("cToken is null : %d\n",cToken != NULL ? 1 : 0);
        if (token_is_type(cToken, SEMICOLON))
        {
            printf("Passed\n\n\n");
            cToken = cToken->next;
            if (cToken)
            {
                struct ast_nodes *next_ast_nodes = malloc(sizeof(struct ast_nodes));
                ast_nodes->next = next_ast_nodes;
                ast_nodes = ast_nodes->next;
                printf("NOTTT ENDD %d %s\n", cToken->type, cToken->value);
            }
            else
            {
                printf("ENDDDD\n");
            }
        }
        else
        {
            printf("Error: Expected semicolon, %d=%s\n", cToken->type, cToken->value);
            while (cToken->next != NULL)
            {
                printf("Token: val: %s %d\n", cToken->value, cToken->type);
                cToken = cToken->next;
            }
            return -1;
        }
    }
    // printf("ARRIVED %d\n", (*original_ast_nodes)->next->current->type);
    return 1;
};
void show_ast_node(struct ast_node *ast_node)
{
    if (!ast_node)
        return;
    // printf("ast_node.type: %d\n", ast_node->type);
    if (ast_node->left)
        show_ast_node(ast_node->left);
    if (ast_node->right)
        show_ast_node(ast_node->right);
}

void show_ast_nodes(struct ast_nodes *ast_nodes)
{
    printf("\n\n\n\nShowing ast_nodes\n\n");
    while (ast_nodes && ast_nodes->current != NULL)
    {
        printf("current.type: %d\n", ast_nodes->current->type);
        if (ast_nodes->current->left)
            show_ast_node(ast_nodes->current->left);
        if (ast_nodes->current->right)
            show_ast_node(ast_nodes->current->right);
        ast_nodes = ast_nodes->next;
        printf("\n");
    }
}

void slice_str(const char *str, char *buffer, size_t start, size_t end)
{
    size_t j = 0;
    for (size_t i = start; i <= end; ++i)
    {
        buffer[j++] = str[i];
    }
    buffer[j] = 0;
}
