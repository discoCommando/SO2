# Funkcja addsuffix dodaje przyrostek, ktory jest jej pierwszym argumentem, do
#                    wszystkich slow na liscie bedacej drugim argumentem
# Regula postaci
#     $(ALL): %: %.o err.o
# mowi, ze dla kazdego pliku z listy $(ALL) ma byc wykonana regula:
#     %: %.o err.o
# gdzie % jest wzorcem nazwy pliku (znak procenta pasuje do dowolnego ciagu 
# znakow) i jednoczesnie zmienna, ktorej wartoscia jest dopasowany ciag znakow.
#
# Zmienne $^, $@ i $< oznaczaja:
#    $^ - nazwy wszystkich plikow wystepujace po ostatnim dwukropku
#		(plikow, od ktorych regula zalezy)
#    $@ - nazwe pliku, ktory stoi przed ostatnim dwukropkiem
#		(pliku, ktory ma zostac odswiezony po zastosowania reguly)
#    $< - nazwe pierwszego pliku wystepujacego po ostatnim dwukropku
#               (pierwszego pliku, od ktorego regula zalezy)

CC      = gcc
CFLAGS  = -Wall -c
LDFLAGS = -Wall -pthread
ALL     = server client
OBJECTS = $(addsuffix .o, $(ALL)) err.o

all: $(ALL) rmKeys

$(ALL): %: %.o err.o
	$(CC) $(LDFLAGS) $^ -o $@

$(OBJECTS): %.o: %.c err.h
	$(CC) $(CFLAGS) $<

rmKeys:
	ipcrm -Q 0x0000029a 
	ipcrm -Q 0x000004d2 
	ipcrm -Q 0x000004d3
	ipcrm -Q 0x0000006c
clean:
	rm -f $(OBJECTS) $(ALL)

