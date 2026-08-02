MK = cd $@ && $(MAKE) && sudo $(MAKE) install

.PHONY: sping

sping:
	$(MK)
