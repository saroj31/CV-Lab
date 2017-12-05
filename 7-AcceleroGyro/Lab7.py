
# coding: utf-8

# In[1]:


import numpy as np
import matplotlib.pyplot as plt
import pandas as pd
import numpy as np
#get_ipython().magic(u'matplotlib inline')

df = pd.read_csv('acc_gyro.txt',delim_whitespace=True)
#print(df[['time','accX','accY']])
print(df.columns)

# ax = df.plot(x='time',y='accX',figsize=(15,3))
# ax = df.plot(x='time',y='accY',figsize=(15,3))
# ax = df.plot(x='time',y='accZ',figsize=(15,3))
#df.plot(x='time',y='accZ',ax=ax,figsize=(15,10))
#ax = df.plot(x='time',y='pitch',ax=ax)
#ax = df.plot(x='time',y='roll',ax=ax)
#df.plot(x='time',y='yaw',ax=ax,figsize=(15,10))
#plt.plot(figsize=(15,10))


# In[2]:


def threshold(dfCol,per=0.1):
    odfCol = dfCol
    maxX = max(dfCol[4:])
    thres = per*maxX

    for i,x in enumerate(dfCol):
        if(abs(x) > abs(thres)):
            odfCol[i] = 1
        else:
            odfCol[i] = 0

    return odfCol

def thresholdVal(dfCol,val=0.1):
    maxX = max(dfCol)
    print(maxX)
    thres = np.var(dfCol)*val

    for i,x in enumerate(dfCol):
        if(abs(x) > abs(thres)):
            
            dfCol[i] = 1
        else:
            dfCol[i] = 0

    return dfCol

def SmoothValMean(dfCol,size=5):
    idf = dfCol.rolling(window=size,center=True).mean()
    return idf

def SmoothValMedian(dfCol,size=5):
    idf = dfCol.rolling(window=size,center=True).median()
    return idf

def RollMax(dfCol,size=5):
    idf = dfCol.rolling(window=size,center=True).max()
    return idf

def makeTimeWindow(indf,attr,MaximSize=5,MeanSize=5,thres=.1,plot=False):
    #make a copy of the accX value
    if(plot):
        indf.plot(x='time',y=attr,figsize=(15,5))

    dy = np.gradient(indf[attr],2)
    indf['dy'] = dy
    if(plot):
        indf.plot(x='time',y='dy',figsize=(15,5))

    indf['dy'] = SmoothValMean(indf.dy.copy(),MeanSize)
    indf['thres'] = threshold(indf.dy.copy(),.1)
    if(plot):
        indf.plot(x='time',y='thres',figsize=(15,5))
    indf['window'] = RollMax(indf.thres.copy(),MaximSize)
    #axes = indf.plot(x='time',y='thres',figsize=(15,5))
    if plot:
        axes = indf.plot(x='time',y='window',figsize=(15,5))
        axes.set_ylim([0,2])
    
    return indf.window


# In[3]:


df['smoothX'] = makeTimeWindow(df.copy(),'accX',MeanSize=6,MaximSize=12,thres=.1,plot=False)


# In[4]:


df['smoothY'] = makeTimeWindow(df.copy(),'accY',MeanSize=6,MaximSize=20,thres=.1,plot=False)


# In[5]:


df['smoothZ'] = makeTimeWindow(df.copy(),'accZ',MeanSize=6,MaximSize=20,thres=.1,plot=False)


# In[6]:


df['smoothP'] = makeTimeWindow(df.copy(),'pitch',MeanSize=7,MaximSize=20,thres=.1,plot=False)


# In[7]:


df['smoothR'] = makeTimeWindow(df.copy(),'roll',MeanSize=9,MaximSize=10,thres=.1,plot=False)


# In[8]:


df['smoothYaw'] = makeTimeWindow(df.copy(),'yaw',MeanSize=9,MaximSize=10,thres=.1,plot=False)


# In[9]:


print(df.shape[0])
df['window'] = 0
for i in range(0,df.shape[0]):
    if(df.smoothX[i] == 1 or df.smoothY[i]==1 or df.smoothZ[i]==1 or df.smoothP[i]==1 or df.smoothR[i]==1 or df.smoothYaw[i]==1):
        df.window[i] = 1
    if( i%100 == 0):
        print(i)

        


# In[10]:


print(df.shape[0])
print(df.columns)


# In[11]:



#plt.savefig("myFile")

#ax.set_ylim([0,2])


# In[15]:


import math

columns = ['accX','accY','accZ','pitch','roll','yaw']
result = pd.DataFrame(columns=columns)
#for pitch
tmp_df = df
#tmp_df.plot(x='time',y='window',figsize=(15,1))
tmp_df['grad'] = np.gradient(tmp_df.window)
#tmp_df.plot(x='time',y='grad',figsize=(15,1))

#print(tmp_df.grad)
ix = tmp_df[tmp_df.grad > 0]
iy = tmp_df[tmp_df.grad < 0]

ix = ix[ix.index%2 == 0]
iy = iy[iy.index%2 == 0]

pitch = pd.Series()
for i in range(0,12):
    
    x = df[ix.index[i]:iy.index[i]]
    res_pitch = (np.trapz(x.pitch,x.time))
    pitch.set_value(i,math.degrees(res_pitch))
    #print("index: " + str(ix.index[i]) + "-" + str(iy.index[i]) + "::::  "+str(math.degrees(pitch)))

result.pitch = pitch
print(result['pitch'])


# In[16]:


import math

#for roll
tmp_df = df
tmp_df['grad'] = np.gradient(tmp_df.window)

#print(tmp_df.grad)
ix = tmp_df[tmp_df.grad > 0]
iy = tmp_df[tmp_df.grad < 0]

ix = ix[ix.index%2 == 0]
iy = iy[iy.index%2 == 0]

roll = pd.Series()
for i in range(0,12):
    x = df[ix.index[i]:iy.index[i]]
    res_roll = np.trapz(x.roll,x.time)
    roll.set_value(i,math.degrees(res_roll)) 

result.roll = roll
print(result.roll)


# In[17]:


#for pitch
tmp_df = df
#tmp_df.plot(x='time',y='window',figsize=(15,1))
tmp_df['grad'] = np.gradient(tmp_df.window)
#tmp_df.plot(x='time',y='grad',figsize=(15,1))

#print(tmp_df.grad)
ix = tmp_df[tmp_df.grad > 0]
iy = tmp_df[tmp_df.grad < 0]

ix = ix[ix.index%2 == 0]
iy = iy[iy.index%2 == 0]

yaw = pd.Series()
for i in range(0,12):
    
    x = df[ix.index[i]:iy.index[i]]
    res_yaw = np.trapz(x.yaw,x.time)
    yaw.set_value(i,math.degrees(res_yaw))
    
result.yaw = yaw
print(result.yaw)


# In[18]:


import scipy as sp
from scipy import integrate

#for pitch
tmp_df = df
#tmp_df.plot(x='time',y='window',figsize=(15,1))
tmp_df['grad'] = np.gradient(tmp_df.window)
#tmp_df.plot(x='time',y='grad',figsize=(15,1))

#print(tmp_df.grad)
ix = tmp_df[tmp_df.grad > 0]
iy = tmp_df[tmp_df.grad < 0]

ix = ix[ix.index%2 == 0]
iy = iy[iy.index%2 == 0]

accX = pd.Series()
for i in range(0,12):
    xdist = 0
    x = df[ix.index[i]:iy.index[i]]
    vdist = integrate.cumtrapz(x.accX,x.time,initial=x.time.iloc[0])
    vdist = pd.Series(vdist)
    xdist = np.trapz(vdist,x.time)
    accX.set_value(i,xdist*9.8)

result.accX = accX
print(result.accX)


# In[19]:


import scipy as sp
from scipy import integrate

#for pitch
tmp_df = df
tmp_df['grad'] = np.gradient(tmp_df.window)
#tmp_df.plot(x='time',y='grad',figsize=(15,1))

#print(tmp_df.grad)
ix = tmp_df[tmp_df.grad > 0]
iy = tmp_df[tmp_df.grad < 0]

ix = ix[ix.index%2 == 0]
iy = iy[iy.index%2 == 0]

accY = pd.Series()
for i in range(0,12):

    x = df[ix.index[i]:iy.index[i]]
    vdist = integrate.cumtrapz(x.accY,x.time,initial=x.time.iloc[0])
    vdist = pd.Series(vdist)
    ydist = np.trapz(vdist,x.time)
    accY.set_value(i,ydist*9.8)

result.accY = accY
print(result.accY)


# In[20]:


import scipy as sp
from scipy import integrate

#for pitch
tmp_df = df.copy()
tmp_df['grad'] = np.gradient(tmp_df.window)
#tmp_df.plot(x='time',y='grad',figsize=(15,1))

#print(tmp_df.grad)
ix = tmp_df[tmp_df.grad > 0]
iy = tmp_df[tmp_df.grad < 0]

ix = ix[ix.index%2 == 0]
iy = iy[iy.index%2 == 0]

accZ = pd.Series()
for i in range(0,12):

    x = df[ix.index[i]:iy.index[i]]
    vdist = integrate.cumtrapz(x.accZ,x.time,initial=x.time.iloc[0])
    vdist = pd.Series(vdist)
    zdist = np.trapz(vdist,x.time)
    accZ.set_value(i,zdist*9.8)

result.accZ = accZ
print(result.accZ)


# In[22]:

wd = 60
ht = 40

ax = df.plot(x='time',y='accX',figsize=(wd,ht))
ax = df.plot(x='time',y='accY',figsize=(wd,ht),ax=ax)
ax = df.plot(x='time',y='accZ',figsize=(wd,ht),ax=ax)
ax = df.plot(x='time',y='pitch',figsize=(wd,ht),ax=ax)
ax = df.plot(x='time',y='roll',figsize=(wd,ht),ax=ax)
ax = df.plot(x='time',y='yaw',figsize=(wd,ht),ax=ax)
ax = df.plot(x='time',y='window',figsize=(wd,ht),ax=ax)
plt.show()


#plt.savefig("Window.png")


print(result)
result.to_csv("result.csv")

