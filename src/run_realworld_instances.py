import os

instances = list()
#inserted in a non-decreasing order by the number of vertices
instances.append("highschool.lcip")
instances.append("residense.lcip")
instances.append("innovation.lcip")
instances.append("wiki-vote.lcip")
instances.append("adolescent_health.lcip")

#paremeters to vary
alphas = list()

alphas.append("1")
alphas.append(".5")
alphas.append(".1")

for alpha in alphas: 
    print("alpha = ", alpha)
    print("network \talg \ttime \tnodes \tdualbound \tprimalbound \tgap")
    for i in instances:
        cmd = '../bin/glcip -f ../data/realworld/' + i + ' -a bc -alpha '+ alpha +' | grep -v "Academic license"'
        output = os.popen(cmd).read()
        output = output.replace("\n", "")
        print(i, "BC \t", output)
        cmd = '../bin/glcip -f ../data/realworld/' + i + ' -a bc+ -alpha '+ alpha +' | grep -v "Academic license"'
        output = os.popen(cmd).read()
        output = output.replace("\n", "")
        print(i, "BC+\t", output)
