# include Husky-Makefile-Config
include ../../huskymak.cfg

ifeq ($(DEBUG), 1)
  CFLAGS = -Ih -I$(INCDIR) $(DEBCFLAGS) $(WARNFLAGS)
  LFLAGS = $(DEBLFLAGS)
else
  CFLAGS = -I$(INCDIR) $(OPTCFLAGS) $(WARNFLAGS)
  LFLAGS = $(OPTLFLAGS)
endif

ifeq ($(SHORTNAME), 1)
  LIBS  = -L$(LIBDIR) -lfidoconf -lsmapi
else
  LIBS  = -L$(LIBDIR) -lfidoconfig -lsmapi
endif

CDEFS=-D$(OSTYPE) $(ADDCDEFS)

default: all

configure-it:
	(cd ..; export LIBS="$(LIBS)"; export CC="$(CC)"; export CFLAGS=$(subst -c,,"$(CDEFS) $(CFLAGS)"); export CXXFLAGS=$(subst -c,,"$(CDEFS) $(CFLAGS)"); if [ ~$(GPP)~ = ~~ ]; then export CXX=g++; else export CXX=$(GPP); fi; rm config.cache; ./configure --bindir=$(BINDIR) --sbindir=$(BINDIR) --libexecdir=$(BINDIR) --datadir=$(CFGDIR) --sysconfdir=$(CFGDIR) --sharedstatedir=$(CFGDIR) --localstatedir=$(CFGDIR) --libdir=$(LIBDIR) --includedir=$(INCDIR) --infodir=$(INFODIR) --mandir=$(MANDIR))
	touch configure-it

all: configure-it
	(cd ..; $(MAKE) all;)
	
install:
	(cd ..; $(MAKE) install;)

clean:
	(cd ..; $(MAKE) clean;)

distclean:
	(cd ..; $(MAKE) distclean;)
	rm configure-it
