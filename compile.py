labels = dict()

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


def compile_gt(ast, idx, env):
    buf = ""
    if len(ast) != 3:
        return buf
    buf += "str r0 r19 " + to_hex(idx + 1) + '\n'
    buf += compile_expr(ast[1], idx + 1, env.copy())
    buf += "str r1d r19 " + to_hex(idx + 2) + '\n'
    buf += compile_expr(ast[2], idx + 2, env.copy())
    buf += "ldr r0 r19 " + to_hex(idx + 2) + '\n'
    buf += "sbs r1d r0 r1d\n"
    buf += "nor r1d r1d r1d\n"
    buf += "shf r1d r1d 3f\n"
    buf += "ldr r0 r19 " + to_hex(idx + 1) + '\n'
    return buf


def compile_load(ast, idx, env):
    buf = ""
    if len(ast) != 3:
        return buf
    buf += "str r0 r19 " + to_hex(idx + 1) + '\n'
    buf += compile_expr(ast[1], idx + 1, env.copy())
    buf += "str r1d r19 " + to_hex(idx + 2) + '\n'
    buf += compile_expr(ast[2], idx + 2, env.copy())
    buf += "ldr r0 r19 " + to_hex(idx + 2) + '\n'
    buf += "ldr r1d r0 r1d\n"
    buf += "ldr r0 r19 " + to_hex(idx + 1) + '\n'
    return buf


def compile_store(ast, idx, env):
    buf = ""
    if len(ast) != 3:
        return buf
    buf += "str r0 r19 " + to_hex(idx + 1) + '\n'
    buf += compile_expr(ast[1], idx + 1, env.copy())
    buf += "str r1d r19 " + to_hex(idx + 2) + '\n'
    buf += compile_expr(ast[2], idx + 2, env.copy())
    buf += "ldr r0 r19 " + to_hex(idx + 2) + '\n'
    buf += "str r1d r0 0\n"
    buf += "ldr r0 r19 " + to_hex(idx + 1) + '\n'
    return buf


def compile_function(ast, idx, env):
    #Caller needs to save and restore link registers
    env = dict()
    n = len(ast[1])
    for i in range(n):
        env[ast[1][i]] = i - n - 1
    labels[ast[0]] = ""
    buf = ast[0] + "\n"
    buf += "str r19 r18 1\n" + "ads r18 r18 1\n" + "ads r19 r18 0\n"
    buf += compile_expr(ast[2], 0, env)
    buf +=  "sbs r18 r18 1\n" + "ldr r19 r18 1\n" + "jnc r1F r1B 0\n"
    labels[ast[0]] = buf
    return "ads r1d r1a " + ast[0] + "\n"


def compile_call(ast, idx, env):
    prims = {'+':"ads", '-':"sbs", '&':"and", '|':"orr", '^':"xor", '~':"nor", '><':"shf", '*':'mls', '/':"dvs"}
    sp_prims = {">":compile_gt, ".":compile_store, ",":compile_load}
    if ast[0] in prims:
        return builtin_op(prims[ast[0]], ast, idx, env.copy())
    elif ast[0] in sp_prims:
        return sp_prims[ast[0]](ast, idx, env.copy())
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
    elif ast[0] == ":":
        ret = compile_function(ast[1:], idx, env.copy())
        return ret
    elif ast[0] in labels:
        idx += 1
        ret = ""
        for arg in ast[1:]:
            ret += compile_expr(arg, idx, env.copy())
            ret += "str r1d r19 " + to_hex(idx) + "\n"
            idx += 1
        offset = to_hex(idx)
        ret += "ads r18 r18 " + offset + "\n" # set stack ptr
        ret += "str r1b r18 0\n" # save link register on stack
        ret += "ads r1b r1e 2\n" # put pc + 2 in the link register
        ret += "jnc r1F r1a " + ast[0] + "\n"
        ret += "ldr r1b r18 0 \n" # restore link register
        ret += "sbs r18 r18 " + offset + "\n" # restore stack pointer
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


def comp(string):
    toks = tokenize(string)
    prologue = "ads r1A r1E 0\n" + "ads r18 r1A STACK:\n" + "ads r19 r18 0\n" + "ads r1b r1E 2\n" + "jnc r1f r1a MAIN:\n" + "xor r1F r1F r1F\n"
    asm = "MAIN:\n"
    for expr in toks:
        asm += compile_expr(expr, 0, {})

    asm += "jnc r1f r1b 0\n\n"

    for key in labels.keys():
        prologue += "\n" + labels[key] + "\n"
    prologue += asm + "STACK: 0"

    return prologue
        
        
#c1 = "[let [[y [: fn [x] x]] [z [fn y]]] z]"
#c1 = "[: fn [x y] [+ x y]] [: fn1 [x y] [+ [fn x y] 1]] [: fn2 [x] x] [fn2 [fn 1 2]]"
c1 = "[: fact [x] [if [> x 1] [* x [fact [- x 1]]] 1]] [fact 10]"
#c1 = "[. 10000 deadbeef] [, ffff 1]"
code = comp(c1)
print(code)
