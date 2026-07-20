C     Dummy STRUCTA subroutine (nuclear structure function)
C     Not provided by LHAPDF6; only called for NSET>0 (nuclear PDF)
      SUBROUTINE STRUCTA(XX,QQ,A,UPV,DNV,USEA,DSEA,STR,CHM,BOT,TOP,
     &GLU)
      IMPLICIT DOUBLE PRECISION (A-H,O-Z)
      UPV=0D0
      DNV=0D0
      USEA=0D0
      DSEA=0D0
      STR=0D0
      CHM=0D0
      BOT=0D0
      TOP=0D0
      GLU=0D0
      RETURN
      END
