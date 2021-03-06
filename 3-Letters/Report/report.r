
library(plotly)
library(ggplot2)

myData = read.csv("Obs.csv")

totYes = 151
totNo = 1111

TPR = myData$TP/totYes
FPR = myData$FP/totNo

print(min(FPR))
print(min(TPR))

print(max(FPR))
print(max(TPR))

nTPR = (TPR - min(TPR))/(max(TPR) - min(TPR))
nFPR = (FPR - min(FPR))/(max(FPR) - min(FPR))

plot(nFPR,nTPR,type = "l")

