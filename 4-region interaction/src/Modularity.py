
# coding: utf-8

# In[12]:

#Black
ebb = 0.258
ebw = .035
ewb = .013
ebh = .016
ehb = .012
ebo = .013
eob = .005

ab = ebb + ebw + ewb + ebh + ehb + ebo + eob
print(ab)
print(.323 + .289 - .258)

#Hispanic
ehb = .012
ehh = .157
ehw = .058
ewh = .023
eho = .019
eoh = .007

ah = ehb + ebh + ehh + + ewh + ehw + eho + eoh
print(ah)
print(.247 + .204 -.157)

#White
eww = .306
ewo = .035
eow = .024

aw = ewb + ebw + ewh + ehw + eww + ewo + eow
print(aw)
print(.423 + .377 - .306 )

#other
eoo = .016
ao = eob + ebo + eoh + eho + eow + ewo + eoo
print(ao)
print(.084 + .053 -.016)

Q = ebb - ab**2 + eww - aw**2 + ehh - ah**2 + eoo - ao**2
print(Q)

xx = .258 - .354**2 + .306 - .494**2 + .157 - .293**2 + .016 - .121**2
xx

