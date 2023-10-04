# Path to the shell script present within the "custom" version of the 8cc compiler.
# This script is also provided when you download the ps4jb2/672/src repository.
ROPCOMPILER = 8cc/8cc/python/rop-ps4-8cc

# Path to the compiler.py file provided by Sleirsgovey when you download his
# PS4JB2/672/src repository.
COMPILER = python3 8cc/bad_hoist/rop/compiler.py

# Path to the Gadgets list obtained from dumps, which are pre-dumped and ready at:
# https://github.com/a0zhar/ps4jb2_672dumps/releases/tag/v2
# You can visit https://github.com/a0zhar/ps4jb2_672dumps for the (JS)compiled netcat.c.
GADGETS = 8cc/bad_hoist/dumps/gadgets.txt

# Output for the (JS)compiled netcat.c.
NETCAT_JS = build/netcat.js

# Output for the (JS)compiled jb.c.
JB_JS = build/jb.js

# These two directories are used as flags when building both netcat and jb from source.
PS4_INC := $(wildcard 8cc/ps4/*.c)
LIBROP_INC := $(wildcard 8cc/librop/*.c)

all: $(JB_JS) $(NETCAT_JS)

$(JB_JS): jb.c printf_stub.c krop.rop spray.rop
	@rm -r temp
	@echo "Compiling $@"
	@$(ROPCOMPILER) /dev/stdout $(LIBROP_INC) $(PS4_INC) jb.c krop.rop spray.rop | $(COMPILER) /dev/stdin $(GADGETS) > $(JB_JS)
	@echo "Finished compiling $@"

$(NETCAT_JS): netcat.c
	@rm -r temp
	@echo "Compiling $@"
	@$(ROPCOMPILER) /dev/stdout $(LIBROP_INC) $(PS4_INC) netcat.c | $(COMPILER) /dev/stdin $(GADGETS) > $(NETCAT_JS)
	@echo "Finished compiling $@"

spray.bin: spray.asm
	@echo "Assembling $@"
	@yasm spray.asm -o spray.bin
	@echo "Finished assembling $@"

spray.rop: spray.bin
	@echo "Generating $@"
	@( echo '_spray_bin:'; echo -n 'db '; xxd -i < spray.bin | tr -d '\n'; echo; echo '_spray_end:'; ) > spray.rop
	@echo "Finished generating $@"

clean:
	@echo "Cleaning..."
	@rm -r temp
	@rm -f $(JB_JS) spray.rop spray.bin
	@echo "Cleaned."
