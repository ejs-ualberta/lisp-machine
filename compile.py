def tok(in_str, i):
    ws = {'\t', '\n', ' '}
    prims = {'[', ']'}
    out = []
    while (i < len(in_str)):
        if in_str[i] == '[':
            i, x = tok(in_str, i+1)
            out.append(x)
        elif in_str[i] == ']':
            return i+1, out
        elif in_str[i] in ws:
            i += 1
            continue
        else:
            t = ""
            while in_str[i] not in prims and in_str[i] not in ws:
                t += in_str[i]
                i += 1
            try:
                n = int(t, 16)
                out.append(n)
            except:
                out.append(t)
    return i, out


def tokenize(in_str):
    i, tr = tok(in_str, 0)
    return tr


def to_hex(val):
    if val < 0:
        return "-" + hex(val)[3:]
    else:
        return hex(val)[2:]


def compile_int(val):
    return 'xor r1d r1d r1d\n' + "ads r1d r1d " + to_hex(val) + '\n'


def compile_function(ast, idx, env):
    #Caller needs to save and restore link register.
    buf = "str r19 r18 1\n" + "ads r18 r18 1\n"
    buf += compile_expr(ast, 0, {})
    buf +=  "sbs r18 r18 1\n" + "ldr r19 r18 1\n" + "jnc r1F r1B 0\n"
    return buf

def compile_call(ast, idx, env):
    prims = {'+':"ads", '-':"sbs", '&':"and", '|':"orr", '^':"xor", '~':"nor", '><':"shf", '*':'mls', '/':"dvs"}
    if ast[0] in prims:
        return builtin_op(prims[ast[0]], ast, idx, env.copy())
    elif ast[0] == "let":
        ret = ""
        for pair in ast[1]:
            idx += 1
            env[pair[0]] = idx
            ret += compile_expr(pair[1], idx, env.copy()) + "str r1d r19 " + to_hex(idx) + "\n"
        ret += compile_expr(ast[2], idx, env.copy())
        return ret
    elif ast[0] == "if":
        exp2 = compile_expr(ast[2], idx, env.copy())
        exp3 = compile_expr(ast[3], idx, env.copy())
        ret = compile_expr(ast[1], idx, env.copy())
        ret += "nor r1d r1d r1d\n" + "and r1d r1d 1\n" + "jnc r1d r1e " + to_hex(2 + exp2.count('\n')) + '\n'
        ret += exp2
        ret += "jnc r1f r1e " + to_hex(1 + exp3.count('\n')) + '\n'
        ret += exp3
        return ret
    else:
        pass

def compile_expr(ast, idx, env):
    if type(ast) is int:
        return compile_int(ast)
    elif type(ast) is list:
        if ast == []:
            return 'xor r1d r1d r1d\n'
        return compile_call(ast, idx, env.copy())
    elif ast in env:
        return "ldr r1d r19 " + to_hex(env[ast]) + '\n'
    else:
        pass

    
def builtin_op(op, ast, idx, env):
    buf = ""
    if len(ast) != 3:
        return buf
    buf += "str r0 r19 " + to_hex(idx + 1) + '\n'
    buf += compile_expr(ast[1], idx + 1, env.copy())
    buf += "str r1d r19 " + to_hex(idx + 2) + '\n'
    buf += compile_expr(ast[2], idx + 2, env.copy())
    buf += "ldr r0 r19 " + to_hex(idx + 2) + '\n'
    buf += op + " r1d r0 r1d\n"
    buf += "ldr r0 r19 " + to_hex(idx + 1) + '\n'
    return buf

c1 = "[let [[x 0]] [if x [+ x 1] [/ 10 2]]]"
asm = compile_expr(tokenize(c1)[0], 0, {})
print(asm)
