# Lets just hope this doesn't get evaluated every time..
CC 	= g++



#UnComment this line if your getting errors about deflate and such
MCCPLIB = -lz

#UnComment this line if your getting errors about crypt and such
CRYPT = -lcrypt

OS=$(shell uname | cut -d _ -f 1)
#Uncomment this line if your compiling on a windows system
ifeq (${OS},CYGWIN)
WIN=1
endif

ifndef WIN 
DYNLIB = "-ldl"
endif

#uncomment this line if you want webpage export
WEB = -DWEB

#IMC2 - Comment out to disable IMC2 support
#IMC = 1
#IMCFLAGS = -DIMC -DIMCSMAUG

#Shouldn't need to touch this...
DLLEXP = -export-dynamic

INDENT  = indent
INDENT_FLAGS = -ts4 -nut -bap -bli0 -di10 -l78 -lp -nbc -i8 -sc -cdb -c1 -cd1 -nbfda -npcs -bad -hnl -nce -npsl -bl

#Uncomment the next line if you want request support
#DBUGFLG = -DREQUESTS
DEFINES = -DMCCP -DMXP -DACCOUNT -DOLC_HOMES -DRESTORE -DOLC_SHUTTLE $(WEB)
DEBUG   = -g3

#Miscellaneous compiler options.
ifndef DEBUG
OPT_FLAG = -pipe -Os
endif

C_FLAGS = $(DEFINES) $(IMCFLAGS) $(DBUGFLG) $(DEBUG) $(DLLEXP)
L_FLAGS = $(DYNLIB) $(MCCPLIB) $(CRYPT) -lm

ifndef DEBUG
W_FLAGS = -pedantic -W -Wall -Wformat-security -Winline -Wstrict-prototypes -Wshadow -Wpointer-arith -Wcast-align -Wcast-qual -Wredundant-decls -Wformat-security
endif

C_FILES =   body.c account.c act_comm.c act_info.c act_move.c act_obj.c \
            act_wiz.c alias.c arena.c autobuild.c ban.c bank.c \
            boards.c bounty.c build.c changes.c channels.c clans.c cleanup.c color.c combat.c \
            comm.c comments.c const.c db.c delivery.c designship.c dns.c editor.c \
            fight.c finger.c handler.c hashstr.c homes.c hotboot.c immcomm.c \
            implants.c installations.c interp.c magic.c makeobjs.c mccp.c \
            medic.c misc.c msp.c mud_comm.c mud_prog.c mxp.c occupations.c olc_bounty.c\
            olc-shuttle.c olc.c pfiles.c pilot.c player.c quest.c races.c raceskills.c \
            renumber.c reset.c restore.c save.c shell.c shops.c \
            skills.c smuggling.c space.c space2.c special.c starsystem.c swskills.c \
            tables.c track.c update.c vendor.c wedding.c grid.c greet.c

O_FILES = $(patsubst %.c,o/%.o,$(C_FILES))

H_FILES = $(wildcard *.h) 

ifdef IMC
   C_FILES :=  $(C_FILES) imc.c md5.c
endif

ifdef WEB
   C_FILES := $(C_FILES) web-server.c 
endif

ifdef WIN
   SWR = swr.exe
   RESOLVER = resolver.exe
else
   SWR = swr
   RESOLVER = resolver
endif

#all: .depend
all:
	@make -s swr

dns:
	make -s resolver

swr: $(O_FILES)
	@echo "Making Executable |$(OS)|$(WIN)|"
	@rm -f swr
ifdef WIN
	@dlltool --export-all --output-def swr.def $(O_FILES)
	@dlltool --dllname $(SWR) --output-exp swr.exp --def swr.def
	@$(CC)  $(W_FLAGS) -o $(SWR) $(O_FILES) swr.exp $(L_FLAGS)
else
	@$(CC)  $(DLLEXP) $(W_FLAGS) -o $(SWR) $(O_FILES) $(L_FLAGS)
endif
	@[ -f /usr/bin/bf ] && /usr/bin/bf -s swr && /usr/bin/bf -m swr || true
	@chmod g+w $(SWR)
	@chmod g+w $(O_FILES)

cvs:
	@echo "Updating from CVS"
	@cvs update

svn:
	@echo "Updating from SVN"
	@~/bin/svn update
	
clean:
	@echo Cleaning Files
	@rm -f $(O_FILES)
	@rm -f o/resolver.o


resolver: o/resolver.o
	@rm -f $(RESOLVER)
	@echo Building Resolver
	@$(CC) $(DEFINES) -o $(RESOLVER) o/resolver.o
	@echo Done compiling resolver
	@chmod g+w $(RESOLVER)
	@chmod a+x $(RESOLVER)
	@chmod g+w o/resolver.o

neat:
	$(INDENT) $(INDENT_FLAGS) $(C_FILES) $(H_FILES)

indentclean:
	rm *.c~ *.h~

o/%.o: %.c
	@echo -n "  Compiling $@...";
	$(CC) -c $(C_FLAGS) $< -o $@
	@echo "Done"

.c.o: mud.h
	@$(CC) -c $(C_FLAGS) $<
	
.depend:  $(C_FILES)
	@echo Making Depend
	@rm -f .depend
# For each source, let the compiler run the preprocessor with the -M and -MM
# options, which causes it to output a dependency suitable for make.
	@for source in $(C_FILES); do \
		$(CC) $(DEFINES) $(INCLUDEDIR) $(C_FLAGS) -M -MM -MT o/`basename $$source .c`.o $$source >> .depend ; \
	done

DEPENDEXISTS=$(shell if [ -e .depend ]; then echo -n "1"; fi )
ifdef (DEPENDEXISTS) 
include .depend
endif
