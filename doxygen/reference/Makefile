REFFILES := block flexiblebox image inline listitem rubyasblock rubyasinline rubytext table tablecell tablecol tablerow tablesection

references: ${REFFILES}

%: %.in referencebuilder
	./referencebuilder "$<" | sort | uniq > "$@"

referencebuilder: referencebuilder.cpp
	g++ -Wall -O2 -o $@ $^

clean:
	rm -f referencebuilder ${REFFILES}

.PHONY: references clean
