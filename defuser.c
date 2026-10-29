#include "defuser.h"

#include <inttypes.h>
#include <stdio.h>

#define MAX_FIELDS 8

/*
 * 把 values 中的 count 个整数写成一行“空格分隔的无符号十进制整数”，存入 out。
 * 成功返回 0；count 为 0（还没有实现）、snprintf 出错或结果被截断时返回非零。
 */
static int write_answer(char *out, size_t cap, const uint32_t *values, size_t count) {
    size_t used = 0;
    for (size_t i = 0; i < count; ++i) {
        int size = snprintf(out + used, cap - used, "%s%" PRIu32, i ? " " : "", values[i]);
        if (size < 0 || (size_t)size >= cap - used) return 1;
        used += (size_t)size;
    }
    return count == 0;
}

int solve_phase_1(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    uint32_t low = (uint32_t)challenge;
    uint32_t mid = (uint32_t)(challenge >> 16);
    uint32_t high = (uint32_t)(challenge >> 32);
    uint32_t a = (3u * high + low) ^ 0x3b60u;
    uint32_t b = (mid + 0xc50u) ^ low;
    values[0] = (a ^ 0x121eu) + 8u * b;
    return write_answer(out, cap, values, count);
}

int solve_phase_2(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 6;
    values[0] = 3u * ((challenge >> 36) & 0xffu);
    for (int i = 1; i < 6; i++) {
        uint32_t shift = 16 + 5 * (i - 1);
        uint32_t temp = (challenge >> shift) & 0x3fu;
        values[i] = 5u * (values[i - 1] + (uint32_t)i) - temp;
    }
    return write_answer(out, cap, values, count);
}

int solve_phase_3(uint64_t challenge, char *out, size_t cap) {
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    uint32_t a = (uint32_t)challenge & 7u;
    uint32_t x = (uint32_t)(challenge >> 3) ^ (uint32_t)(challenge >> 32);
    values[0] = a;
    switch (a) {
        case 0: values[1] = 3u * x + 0x671bu; break;
        case 1: values[1] = 5u * x + 0x1946au; break;
        case 2: values[1] = 5u * x + 0x4b479u; break;
        case 3: values[1] = (x ^ 0xc26eu) + (x >> 1); break;
        case 4: values[1] = (4u * x) ^ 0xb24eu; break;
        case 5: values[1] = (x ^ 0x3862u) + (x >> 2); break;
        case 6: values[1] = 3u * x + 0x2d4eu; break;
        case 7: values[1] = (2u * x) ^ 0xf035u; break;
    }
    return write_answer(out, cap, values, count);
}

static uint32_t p4_walk(uint32_t target, uint32_t low,
                               uint32_t high, uint32_t key)
{
    uint32_t mid = low + ((high - low) >> 1);

    if (target == mid)
        return (mid + 0xaeu) ^ key;

    uint32_t temp = mid + key;

    if (target < mid) {
        uint32_t result = p4_walk(target, low, mid - 1, key);
        return ((3u * result) ^ temp) ^ 0x12u;
    }
    else {
        uint32_t result = p4_walk(target, mid + 1, high, key);
        return ((3u * result) ^ temp) ^ 0x10u;
    }
}

int solve_phase_4(uint64_t challenge, char *out, size_t cap)
{
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    uint32_t target = (uint32_t)challenge & 0x7fu;
    uint32_t key = (uint32_t)(challenge >> 7)
                 ^ (uint32_t)(challenge >> 32);
    values[0] = p4_walk(target, 0, 127, key);
    return write_answer(out, cap, values, count);
}
int solve_phase_5(uint64_t challenge, char *out, size_t cap)
{
typedef struct {
        int next;
        uint32_t value;
        uint32_t key;
    } P5Node;
    static const P5Node p5_nodes[12] = {
        { 10,     0x071cu,    0xc3f5u },  
        {  4,     0x021au,    0x1073u },  
        {  7,     0x0a47u,    0xdb1fu },  
        {  0,     0x0349u,    0xe266u },  
        {  5,     0x0548u,    0x6689u },  
        {  9,     0x005cu,    0xa66bu },  
        { -1,     0x0871u,    0xaccau },  
        {  3,     0x0b67u,    0x37aau },  
        {  2,     0x0654u,    0x99d9u },  
        {  8,     0x017bu,    0xa3a0u },  
        {  6,     0x0935u,    0x5dbdu },  
        {  1,     0x0476u,    0xfafeu },  
    };
    static const int heads[8] = {
        11, 1, 4, 5, 9, 8, 2, 7
    };
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 2;
    uint32_t index = (uint32_t)challenge & 7u;
    uint32_t steps = (uint32_t)(challenge >> 3) & 3u;
    int p = heads[index];
    uint32_t result = (uint32_t)(challenge >> 6)
                    ^ (uint32_t)(challenge >> 32);
    result ^= p5_nodes[p].key;
    result ^= 0x2865u;
    result += p5_nodes[p].value;
    for (uint32_t i = 0; i < steps; i++) {
        p = p5_nodes[p].next;

        result ^= p5_nodes[p].key;
        result += p5_nodes[p].value;
    }
    values[0] = result;
    values[1] = p5_nodes[p].value;
    return write_answer(out, cap, values, count);
}


static uint32_t p6_apply(int op, uint32_t x)
{
    switch (op) {
        case 0:
            return (x >> 2) | (x << 30); 
        case 1:
            return ((x >> 7) ^ x) + 0xf8fcu;
        case 2:
            return ((x << 7) + 0x1aa7u) ^ (3u * x);
        case 3:
            return (x ^ 0xc09cu) + (x << 5);
        case 4:
            return ((x << 1) | (x >> 31)) ^ 0x3362u;  
        case 5:
            return (x + 0x9cf4u) ^ (x >> 5);
        default:
            return 0;
    }
}

int solve_phase_6(uint64_t challenge, char *out, size_t cap)
{
    uint32_t values[MAX_FIELDS] = {0};
    size_t count = 1;
    static const int ops[8] = {3, 5, 0, 2, 1, 2, 1, 4};
    uint32_t x = (uint32_t)(challenge >> 6)
               ^ (uint32_t)(challenge >> 32);
    uint32_t k0 = (uint32_t)challenge & 7u;
    uint32_t k1 = (uint32_t)(challenge >> 3) & 7u;
    uint32_t y = p6_apply(ops[k0], x);
    y ^= 0x9864u;
    uint32_t z = p6_apply(ops[k1], y);
    values[0] = z + 0x9ff5u;
    return write_answer(out, cap, values, count);
}

int solve_secret(uint64_t challenge, uint64_t context, char *out, size_t cap) {
   // 第一个整数：取 context 的最低两位
    uint32_t a = (uint32_t)context & 3u;

    // 分别将 challenge 和 context 的高低32位异或
    uint32_t x = (uint32_t)(challenge >> 32)
               ^ (uint32_t)challenge;

    uint32_t y = (uint32_t)(context >> 32)
               ^ (uint32_t)context;

    // 计算中间参数
    uint32_t k = (a * 0x9a55u)
               ^ (uint32_t)(context >> 16);

    // secret_mix：循环右移3位，再异或
    uint32_t t = x ^ y;
    uint32_t mix = ((t >> 3) | (t << 29)) ^ 0x5bf2u;

    // 第二个整数
    uint32_t b = mix ^ k;

    // 第三个整数
    uint32_t c = b ^ (y + a + 0xc1a7u + (x >> 3));

    // 前32个字符 + OPEN + 三个整数
    int n = snprintf(out, cap,
        "AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAOPEN%u %u %u",
        a, b, c);

    if (n < 0 || (size_t)n >= cap)
        return 1;

    return 0;
}