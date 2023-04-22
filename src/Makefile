CC      = g++
PROF    =

# colors.
L_BLUE    = \033[1;34m
L_LBLUE   = \033[1;36m
L_GREEN   = \033[1;32m
L_WHITE   = \033[1;37m
L_NRM     = \033[0m
L_YELLOW  = \033[1;33m

#Uncomment to compile in Cygwin
#CYGWIN = -DCYGWIN

#Uncomment the line below if you are getting undefined references to dlsym, dlopen, and dlclose.
#Comment it out if you get errors about ldl not being found.
NEED_DL = -ldl

#Some systems need this for dynamic linking to work.
EXPORT_SYMBOLS = -export-dynamic

#Uncomment the line below if you are getting warnings about undefined math functions
NEED_MATH = -lm

C_FLAGS = -g2 $(W_FLAGS) $(PROF) $(EXPORT_SYMBOLS)
L_FLAGS = $(PROF) $(NEED_MATH) -lz $(NEED_DL)
#D_FLAGS : For the DNS Slave process. No need in linking all the extra libs for this.
D_FLAGS = -g2 -O $(PROF) $(SOLARIS_LINK)

W_FLAGS = -std=c++1z -Wall -Wshadow -Wformat-security -Wpointer-arith -Wcast-align -Wredundant-decls

C_FILES = act_comm.cpp act_info.cpp act_move.cpp act_obj.cpp act_wiz.cpp arena.cpp bits.cpp board.cpp bounty.cpp \
	  build.cpp clans.cpp color.cpp comm.cpp comments.cpp const.cpp db.cpp dns.cpp fight.cpp freeze.cpp grub.cpp \
	  handler.cpp hashstr.cpp hiscores.cpp host.cpp hotboot.cpp immcomm.cpp interp.cpp land.cpp magic.cpp \
	  mail.cpp makeobjs.cpp mccp.cpp memory.cpp misc.cpp msweapons.cpp mud_comm.cpp mud_prog.cpp pfiles.cpp \
	  player.cpp relationship.cpp renumber.cpp reset.cpp save.cpp sha256.cpp shops.cpp skills.cpp space.cpp \
          special.cpp swskills.cpp tables.cpp track.cpp update.cpp

O_FILES := $(patsubst %.cpp,o/%.o,$(C_FILES))

H_FILES = $(wildcard *.h)

all:
	@$(MAKE) -s gwom
	@$(MAKE) -s resolver

# pull in dependency info for *existing* .o files
-include dependencies.d

ifdef CYGWIN
gwom: dependencies.d o $(O_FILES)
	@rm -f gwom.exe
	dlltool --export-all --output-def gwom.def $(O_FILES)
	dlltool --dllname gwom.exe --output-exp gwom.exp --def gwom.def
	$(CC) -o gwom.exe $(O_FILES) gwom.exp $(L_FLAGS) $(NOCRYPT)
	@echo "$(L_BLUE)[- $(L_GREEN) Done compiling mud.$(L_NRM)";
	@chmod g+w gwom.exe
	@chmod a+x gwom.exe
	@chmod g+w $(O_FILES)

clean:
	@rm -f o/*.o gwom.exe dependencies.d resolver.exe *~
	@rmdir o

else
gwom: dependencies.d o $(O_FILES)
	@rm -f gwom
	$(CC) -export-dynamic -o gwom $(O_FILES) $(L_FLAGS) $(NOCRYPT)
	@echo "$(L_BLUE)[- $(L_GREEN) Done compiling mud.$(L_NRM)";
	@chmod g+w gwom
	@chmod a+x gwom
	@chmod g+w $(O_FILES)

clean:
	@rm -f o/*.o gwom dependencies.d resolver *~
	@rmdir o

endif

o:
	mkdir o

dependencies.d: $(C_FILES) $(H_FILES)
	@echo "$(L_BLUE)[- $(L_YELLOW)Generating dependency file ...$(L_NRM)";
	@$(CC) -MM $(C_FLAGS) $(C_FILES) > dependencies.d
	@perl -pi -e 's.^([a-z]).o/$$1.g' dependencies.d
	@chmod ug+w dependencies.d

resolver: o/resolver.o
	@rm -f resolver
	$(CC) $(D_FLAGS) -o resolver o/resolver.o
	@echo "$(L_BLUE)[- $(L_LBLUE) Done compiling DNS resolver.$(L_NRM)";
	@chmod g+w resolver
	@chmod a+x resolver
	@chmod g+w o/resolver.o

indent:
	indent -ts4 -nut -nsaf -nsai -nsaw -npcs -npsl -ncs -nbc -bls -prs -bap -cbi0 -cli3 -bli0 -l125 -lp -i3 -cdb -c1 -cd1 -sc -pmt $(C_FILES)
	indent -ts4 -nut -nsaf -nsai -nsaw -npcs -npsl -ncs -nbc -bls -prs -bap -cbi0 -cli3 -bli0 -l125 -lp -i3 -cdb -c1 -cd1 -sc -pmt $(H_FILES)

indentclean:
	@rm *.cpp~ *.h~

o/%.o: %.cpp
	@echo "$(L_BLUE)[- $(L_WHITE)  Compiling $@....$(L_NRM)";
	$(CC) -c $(C_FLAGS) $< -o $@

#.cpp.o: mud.h
#	$(CC) -c $(C_FLAGS) $<
