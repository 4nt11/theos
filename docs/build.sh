#!/bin/bash

DOCNAME="general_docs"

cat <<EOF > $DOCNAME.md
---
title: "theos"
author: "me."
template: letter
date: "\\today"
standalone: true
header-includes:
  - \usepackage{enumitem}
  - \setlistdepth{20}
  - \renewlist{itemize}{itemize}{20}
  - \renewlist{enumerate}{enumerate}{20}
  - \setlist[itemize]{label=$\cdot$}
  - \setlist[itemize,1]{label=\textbullet}
  - \setlist[itemize,2]{label=--}
  - \setlist[itemize,3]{label=*}
  #- \renewcommand{\includegraphics}[2][]{}
  - \newcommand{\x}[1]{\textbackslash x#1}
  - \makeatletter
  - \def\verbatim{\tiny\@verbatim \frenchspacing\@vobeyspaces \@xverbatim}
  - \makeatother

geometry: margin=1in
output:
  rmarkdown::pdf_document:
      keep_tex: yes
---

EOF

for i in {0..7};
do
	sed 's/\\/\\\\/g' "$i"-* >> $DOCNAME.md
done

pandoc --toc --toc-depth=6 -V fontsize=10pt \
	--pdf-engine=xelatex --from=markdown+tex_math_dollars+tex_math_single_backslash \
	-o $DOCNAME.pdf $DOCNAME.md --verbose -s --embed-resources=false --fail-if-warnings=false \
	--sandbox=true 
