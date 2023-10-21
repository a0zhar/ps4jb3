// Copyright 2012 Rui Ueyama. Released under the MIT license.

#include "8cc.h"

static char *decorate_int(char *name, Type *ty) {
    char *u = (ty->usig) ? "u" : "";
    if (ty->bitsize > 0)
        return format("%s%s:%d:%d", u, name, ty->bitoff, ty->bitoff + ty->bitsize);

    return format("%s%s", u, name);
}

static char *do_ty2s(Dict *dict, Type *ty) {
    if (!ty) return "(nil)";
    switch (ty->kind) {
        case KIND_VOID:    return "void";
        case KIND_BOOL:    return "_Bool";
        case KIND_CHAR:    return decorate_int("char", ty);
        case KIND_SHORT:   return decorate_int("short", ty);
        case KIND_INT:     return decorate_int("int", ty);
        case KIND_LONG:    return decorate_int("long", ty);
        case KIND_LLONG:   return decorate_int("llong", ty);
        case KIND_FLOAT:   return "float";
        case KIND_DOUBLE:  return "double";
        case KIND_LDOUBLE: return "long double";
        case KIND_PTR:     return format("*%s", do_ty2s(dict, ty->ptr));
        case KIND_ARRAY:   return format("[%d]%s", ty->len, do_ty2s(dict, ty->ptr));
        case KIND_STRUCT:
            char *kind = ty->is_struct ? "struct" : "union";
            if (dict_get(dict, format("%p", ty)))
                return format("(%s)", kind);

            dict_put(dict, format("%p", ty), (void *)1);
            if (ty->fields) {
                Buffer *b = make_buffer();
                if (b == NULL || b->body == NULL)
                    return NULL; // Handle error

                buf_printf(b, "(%s", kind);
                Vector *keys = dict_keys(ty->fields);
                for (int i = 0, len = vec_len(keys); i < len; i++) {
                    char *key = vec_get(keys, i);
                    Type *fieldtype = dict_get(ty->fields, key);
                    buf_printf(b, " (%s)", do_ty2s(dict, fieldtype));
                }
                buf_printf(b, ")");
                return buf_body(b);
            }
            break;
        case KIND_FUNC:
            Buffer *b = make_buffer();
            buf_printf(b, "(");
            if (ty->params) {
                for (int i = 0, len = vec_len(ty->params); i < len; i++) {
                    if (i > 0)
                        buf_printf(b, ",");
                    Type *t = vec_get(ty->params, i);
                    buf_printf(b, "%s", do_ty2s(dict, t));
                }
            }
            buf_printf(b, ")=>%s", do_ty2s(dict, ty->rettype));
            return buf_body(b);
            break;
        default: return format("(Unknown ty: %d)", ty->kind);
    };
    return NULL;
}

char *ty2s(Type *ty) {
    return do_ty2s(make_dict(), ty);
}

static void uop_to_string(Buffer *b, char *op, Node *node) {
    buf_printf(b, "(%s %s)", op, node2s(node->operand));
}

static void binop_to_string(Buffer *b, char *op, Node *node) {
    buf_printf(b, "(%s %s %s)", op, node2s(node->left), node2s(node->right));
}

static void a2s_declinit(Buffer *b, Vector *initlist) {
    for (int i = 0, len = vec_len(initlist); i < len; i++) {
        if (i > 0) buf_printf(b, " ");

        Node *init = vec_get(initlist, i);
        buf_printf(b, "%s", node2s(init));
    }
}
void handle_generic(Buffer *b, Node *node) {
    if (b == NULL) return;

    switch (node->ty->kind) {
        case KIND_CHAR:
            if (node->ival == '\n')      buf_printf(b, "'\n'");
            else if (node->ival == '\\') buf_printf(b, "'\\\\'");
            else if (node->ival == '\0') buf_printf(b, "'\\0'");
            else buf_printf(b, "'%c'", node->ival);
            break;
        case KIND_INT:buf_printf(b, "%d", node->ival); break;
        case KIND_LONG:buf_printf(b, "%ldL", node->ival); break;
        case KIND_LLONG:buf_printf(b, "%lldL", node->ival); break;
        case KIND_FLOAT:
        case KIND_DOUBLE:
        case KIND_LDOUBLE:
            buf_printf(b, "%f", node->fval);
            break;
        case KIND_ARRAY:buf_printf(b, "\"%s\"", quote_cstring(node->sval)); break;
        default: error("internal error"); break;
    }
}

// REWRITING THIS FUNCTION TO MAKE IT EASIER TO MAINTAIN AND LESS MESSY
static void do_node2s(Buffer *buf, Node *node) {
    if (!node) {
        buf_printf(buf, "(nil)");
        return;
    }
    switch (node->kind) {
        case AST_LITERAL: handle_generic(buf, node); break;
        case AST_LABEL:   buf_printf(buf, "%s:", node->label); break;
        case AST_LVAR:
            buf_printf(buf, "lv=%s", node->varname);
            if (node->lvarinit) {
                buf_printf(buf, "(");
                a2s_declinit(buf, node->lvarinit);
                buf_printf(buf, ")");
            }
            break;
        case AST_GVAR: buf_printf(buf, "gv=%s", node->varname); break;
        case AST_FUNCALL:
        case AST_FUNCPTR_CALL:
            buf_printf(buf, "(%s)%s(", ty2s(node->ty), node->kind == AST_FUNCALL ? node->fname : node2s(node));

            for (int i = 0, len = vec_len(node->args); i < len; i++) {
                if (i > 0) buf_printf(buf, ",");
                buf_printf(buf, "%s", node2s(vec_get(node->args, i)));
            }
            buf_printf(buf, ")");
            break;
        case AST_FUNCDESG: buf_printf(buf, "(funcdesg %s)", node->fname); break;
        case AST_FUNC:
            buf_printf(buf, "(%s)%s(", ty2s(node->ty), node->fname);
            for (int i = 0, len = vec_len(node->params); i < len; i++) {
                if (i > 0) buf_printf(buf, ",");
                Node *param = vec_get(node->params, i);
                buf_printf(buf, "%s %s", ty2s(param->ty), node2s(param));
            }
            buf_printf(buf, ")");
            do_node2s(buf, node->body);
            break;
        case AST_GOTO: buf_printf(buf, "goto(%s)", node->label); break;
        case AST_DECL:
            buf_printf(buf, "(decl %s %s", ty2s(node->declvar->ty), node->declvar->varname);
            if (node->declinit) {
                buf_printf(buf, " ");
                a2s_declinit(buf, node->declinit);
            }
            buf_printf(buf, ")");
            break;
        case AST_INIT:
            buf_printf(buf, "%s@%d", node2s(node->initval), node->initoff, ty2s(node->totype));
            break;
        case AST_CONV:buf_printf(buf, "(conv %s=>%s)", node2s(node->operand), ty2s(node->ty)); break;
        case AST_IF:
            buf_printf(buf, "(if %s %s", node2s(node->cond), node2s(node->then));
            if (node->els) buf_printf(buf, " %s", node2s(node->els));
            buf_printf(buf, ")");
            break;
        case AST_TERNARY:
            buf_printf(buf, "(? %s %s %s)",
                node2s(node->cond),
                node2s(node->then),
                node2s(node->els));
            break;
        case AST_RETURN:buf_printf(buf, "(return %s)", node2s(node->retval)); break;
        case AST_COMPOUND_STMT:
            buf_printf(buf, "{");
            for (int i = 0, len = vec_len(node->stmts); i < len; i++) {
                do_node2s(buf, vec_get(node->stmts, i));
                buf_printf(buf, ";");
            }
            buf_printf(buf, "}");
            break;
        case AST_STRUCT_REF:
            do_node2s(buf, node->struc);
            buf_printf(buf, ".");
            buf_printf(buf, node->field);
            break;
        case AST_ADDR:  uop_to_string(buf, "addr", node); break;
        case AST_DEREF: uop_to_string(buf, "deref", node); break;
        case OP_SAL:    binop_to_string(buf, "<<", node); break;
        case OP_SAR:
        case OP_SHR:
            binop_to_string(buf, ">>", node);
            break;
        case OP_GE:       binop_to_string(buf, ">=", node); break;
        case OP_LE:       binop_to_string(buf, "<=", node); break;
        case OP_NE:       binop_to_string(buf, "!=", node); break;
        case OP_PRE_INC:  uop_to_string(buf, "pre++", node); break;
        case OP_PRE_DEC:  uop_to_string(buf, "pre--", node); break;
        case OP_POST_INC: uop_to_string(buf, "post++", node); break;
        case OP_POST_DEC: uop_to_string(buf, "post--", node); break;
        case OP_LOGAND:   binop_to_string(buf, "and", node); break;
        case OP_LOGOR:    binop_to_string(buf, "or", node); break;
        case OP_A_ADD:    binop_to_string(buf, "+=", node); break;
        case OP_A_SUB:    binop_to_string(buf, "-=", node); break;
        case OP_A_MUL:    binop_to_string(buf, "*=", node); break;
        case OP_A_DIV:    binop_to_string(buf, "/=", node); break;
        case OP_A_MOD:    binop_to_string(buf, "%=", node); break;
        case OP_A_AND:    binop_to_string(buf, "&=", node); break;
        case OP_A_OR:     binop_to_string(buf, "|=", node); break;
        case OP_A_XOR:    binop_to_string(buf, "^=", node); break;
        case OP_A_SAL:    binop_to_string(buf, "<<=", node); break;
        case OP_A_SAR:
        case OP_A_SHR:
            binop_to_string(buf, ">>=", node);
            break;
        case '!': uop_to_string(buf, "!", node);   break;
        case '&': binop_to_string(buf, "&", node); break;
        case '|': binop_to_string(buf, "|", node); break;
        case OP_CAST:
            buf_printf(buf, "((%s)=>(%s) %s)", ty2s(node->operand->ty), ty2s(node->ty), node2s(node->operand));
            break;
        case OP_LABEL_ADDR: buf_printf(buf, "&&%s", node->label); break;
        default:
            if (node->kind == OP_EQ) buf_printf(buf, "(== ");
            else buf_printf(buf, "(%c ", node->kind);
            buf_printf(buf, "%s %s)", node2s(node->left), node2s(node->right));
            break;
    }
}

char *node2s(Node *node) {
    Buffer *b = make_buffer();
    if (b == NULL) return NULL;

    do_node2s(b, node);
    return buf_body(b);
}

static char *encoding_prefix(int enc) {
    switch (enc) {
        case ENC_CHAR16: return "u";
        case ENC_CHAR32: return "U";
        case ENC_UTF8:   return "u8";
        case ENC_WCHAR:  return "L";
        default: /* unknown */ break;
    };
    return "(unknown encoding)";
}

char *tok2s(Token *tok) {
    if (!tok) return "(null)";

    switch (tok->kind) {
        case TIDENT:       return tok->sval;
        case TCHAR:        return format("%s'%s'", encoding_prefix(tok->enc), quote_char(tok->c));
        case TNUMBER:      return tok->sval;
        case TSTRING:      return format("%s\"%s\"", encoding_prefix(tok->enc), quote_cstring(tok->sval));
        case TEOF:         return "(eof)";
        case TINVALID:     return format("%c", tok->c);
        case TNEWLINE:     return "(newline)";
        case TSPACE:       return "(space)";
        case TMACRO_PARAM: return "(macro-param)";
        case TKEYWORD:
            switch (tok->id) {
                #define op(id, str)         case id: return str;
                #define keyword(id, str, _) case id: return str;
                #include "keyword.inc"
                #undef keyword
                #undef op
                default: return format("%c", tok->id);
            };
            break;
        default: /* unknown token type*/ break;
    }
    error("internal error: unknown token kind: %d", tok->kind);
}
