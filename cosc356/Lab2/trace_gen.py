import random

# Configuration
MEMORY_SIZE = 1024 * 1024 # 1 MB simulated memory
NUM_ACCESSES = 10000 # Total accesses per trace

def generate_linear () :
    """Generates␣a␣sequential␣linear␣scan␣(good␣spatial␣locality)."""
    with open ("linear.trace", "w") as f:
        for i in range (0, NUM_ACCESSES * 4, 4) :
            addr = 0x40000000 + (i % MEMORY_SIZE)
            f.write(f"0x{addr:08x}\n")

def generate_random () :
    """Generates␣completely␣random␣accesses␣(poor␣locality)."""
    random.seed (42) # Fixed seed for reproducibility
    with open ("random.trace", "w") as f:
        for i in range(NUM_ACCESSES):
            addr = 0x40000000 + random.randint(0 , MEMORY_SIZE)
            # Align to 4 bytes
            addr = addr & ~3
            f.write(f"0x{addr:08x}\n")

if __name__ == "__main__":
    generate_linear()
    generate_random()
    print ("Generated␣linear.trace␣and␣random.trace")
