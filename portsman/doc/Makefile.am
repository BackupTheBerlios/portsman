dist_man1_MANS = portsman.1
dist_man5_MANS = portsmanrc.5
pmanhelpdir = $(prefix)/share/doc/portsman
dist_pmanhelp_DATA = portsman.help
NROFF = nroff -man

portsman.1 : 
	$(NROFF) portsman.man > portsman.1

portsmanrc.5 : 
	$(NROFF) portsmanrc.man > portsmanrc.5

clean-local:
	rm -rf *.1 *.5
