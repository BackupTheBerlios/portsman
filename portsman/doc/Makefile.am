dist_man1_MANS = portsman.1
dist_man5_MANS = portsmanrc.5
NROFF = nroff -man

portsman.1 : 
	$(NROFF) portsman.man > portsman.1

portsmanrc.5 : 
	$(NROFF) portsmanrc.man > portsmanrc.5

clean-local:
	rm -f *.1 *.5
