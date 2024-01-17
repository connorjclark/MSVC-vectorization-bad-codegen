#include <cassert>
#include <cstdint>
#include <random>
#include <stdio.h>

typedef uint64_t BitWord;

std::mt19937_64 gen64;

struct Block {
  BitWord *in;
  BitWord *out;
  BitWord *gen;
  BitWord *kill;
};

BitWord *mk(size_t len) { return new BitWord[len]; }

static bool op(BitWord *dst, const BitWord *a, const BitWord *b,
               const BitWord *c, uint32_t n) {
  BitWord changed = 0;

  // If `#pragma loop(no_vector)` is placed here, the bug does not occur.
  for (uint32_t i = 0; i < n; i++) {
    BitWord before = dst[i];
    BitWord after = (a[i] | b[i]) & ~c[i];

    dst[i] = after;
    changed |= (before ^ after);
  }

  return changed != 0;
}

int main() {
  Block block{};

  // Create some int arrays of size n.
  // If this is less than 4, nothing is vectorized (assuming 64 bit arch with
  // SSE), and the bug does not occur.
  size_t n = 4;
  block.in = mk(n);
  block.out = mk(n);
  block.gen = mk(n);
  block.kill = mk(n);

  // Create a random but consistent array of ints.
  for (size_t i = 0; i < n; i++) {
    block.in[i] = rand();
    block.out[i] = rand();
    block.gen[i] = rand();
    block.kill[i] = rand();
  }

  // This operation sets each element of `in` to some value based only on the
  // same index element of the other arrays. It is naturally vectorizable, so
  // MSVC auto vectorizes it.
  bool changed = op(block.in, block.out, block.gen, block.kill, n);
  // Debug mode generates the correct result, but Release mode does not.
  bool failed = block.in[0] != 0x000000000000183B;
  printf("in[0]: %p\n", block.in[0]);
  // If this line is removed, `changed` is optimized away and the bug does not
  // occur.
  printf("changed: %d\n", changed);
  printf("failed: %d\n", failed);
  return failed ? 1 : 0;
}
