FONTGFXDIR := graphics/fonts

$(FONTGFXDIR)/latin_small.latfont: $(FONTGFXDIR)/latin_small.png
	$(GFX) $< $@

$(FONTGFXDIR)/japanese_small.fwjpnfont: $(FONTGFXDIR)/japanese_small.png
	$(GFX) $< $@

$(FONTGFXDIR)/japanese_tall.fwjpnfont: $(FONTGFXDIR)/japanese_tall.png
	$(GFX) $< $@

$(FONTGFXDIR)/latin_normal.latfont: $(FONTGFXDIR)/latin_normal.png
	$(GFX) $< $@

$(FONTGFXDIR)/japanese_normal.fwjpnfont: $(FONTGFXDIR)/japanese_normal.png
	$(GFX) $< $@

$(FONTGFXDIR)/latin_male.latfont: $(FONTGFXDIR)/latin_male.png
	$(GFX) $< $@

$(FONTGFXDIR)/japanese_male.fwjpnfont: $(FONTGFXDIR)/japanese_male.png
	$(GFX) $< $@

$(FONTGFXDIR)/latin_female.latfont: $(FONTGFXDIR)/latin_female.png
	$(GFX) $< $@

$(FONTGFXDIR)/japanese_female.fwjpnfont: $(FONTGFXDIR)/japanese_female.png
	$(GFX) $< $@

$(FONTGFXDIR)/braille.fwjpnfont: $(FONTGFXDIR)/braille.png
	$(GFX) $< $@

$(FONTGFXDIR)/japanese_bold.fwjpnfont: $(FONTGFXDIR)/japanese_bold.png
	$(GFX) $< $@
