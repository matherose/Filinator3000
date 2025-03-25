# Makefile pour Filinator
# Pour construire:   make
# Pour nettoyer:     make clean
#
# Vous pouvez remplacer des variables comme 'TARGET' via la ligne de commande, par exemple:
#   make TARGET=filinator.exe

CC = clang
TARGET = filinator
SRC = filinator.c

MAJOR = 0
MINOR = 4
PATCH = 0

# Flags pour GNU99 et optimisation élevée.
# Sur les systèmes non-Windows, nous incluons -march=native et -flto.
# Vous pouvez remplacer cette variable depuis la ligne de commande si nécessaire.
CFLAGS = -std=gnu99 -O3 -march=native -flto -Wall -Wextra -pedantic

# Cible par défaut
all: $(TARGET)

# Compilation de filinator
$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

# Cible de nettoyage
clean:
	rm -f $(TARGET)

.PHONY: all clean

release: all clean
	@echo "Version: $(MAJOR).$(MINOR).$(PATCH)"
	@echo "CFLAGS: $(CFLAGS)"
	@echo "CC: $(CC)"
	@echo "TARGET: $(TARGET)"
	@echo "SRC: $(SRC)"

	# Create tag and push it
	git add .
	git commit -m "Version $(MAJOR).$(MINOR).$(PATCH)"
	git push origin main
	git tag -a v$(MAJOR).$(MINOR).$(PATCH) -m "Version $(MAJOR).$(MINOR).$(PATCH)"
	git push origin v$(MAJOR).$(MINOR).$(PATCH)
