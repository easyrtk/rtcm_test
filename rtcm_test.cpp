// rtcm_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "rtklib.h"


static void decode_rtcm(rtcm_t* rtcm)
{
    if (rtcm->nbyte < 3) return;
    rtcm->len=getbitu(rtcm->buff,14,10)+3; /* length without parity */
    if (rtcm->nbyte<rtcm->len+3) return;
    /* check parity */
    if (rtk_crc24q(rtcm->buff,rtcm->len)!=getbitu(rtcm->buff,rtcm->len*8,24)) {
        trace(2,"rtcm3 parity error: len=%d\n",rtcm->len);
        return;
    }
    /* decode rtcm3 message */
    decode_rtcm3(rtcm);
}
/* write rtcm3 msm to stream -------------------------------------------------*/
static void write_rtcm3_msm(rtcm_t *out, int msg, int sync, FILE *fOUT)
{
    obsd_t *data,buff[MAXOBS];
    int i,j,n,ns,sys,nobs,code,nsat=0,nsig=0,nmsg,mask[MAXCODE]={0};
    
    if      (1071<=msg&&msg<=1077) sys=SYS_GPS;
    else if (1081<=msg&&msg<=1087) sys=SYS_GLO;
    else if (1091<=msg&&msg<=1097) sys=SYS_GAL;
    else if (1101<=msg&&msg<=1107) sys=SYS_SBS;
    else if (1111<=msg&&msg<=1117) sys=SYS_QZS;
    else if (1121<=msg&&msg<=1127) sys=SYS_CMP;
    else return;
    
    data=out->obs.data;
    nobs=out->obs.n;
    
    /* count number of satellites and signals */
    for (i=0;i<nobs&&i<MAXOBS;i++) {
        if (satsys(data[i].sat,NULL)!=sys) continue;
        nsat++;
        for (j=0;j<NFREQ+NEXOBS;j++) {
            if (!(code=data[i].code[j])||mask[code-1]) continue;
            mask[code-1]=1;
            nsig++;
        }
    }
    if (nsig<=0||nsig>64) return;
    
    /* pack data to multiple messages if nsat x nsig > 64 */
    ns=64/nsig;         /* max number of sats in a message */
    nmsg=(nsat-1)/ns+1; /* number of messages */
    
    out->obs.data=buff;
    
    for (i=j=0;i<nmsg;i++) {
        for (n=0;n<ns&&j<nobs&&j<MAXOBS;j++) {
            if (satsys(data[j].sat,NULL)!=sys) continue;
            out->obs.data[n++]=data[j];
        }
        out->obs.n=n;
        
        if (gen_rtcm3(out,msg,i<nmsg-1?1:sync)&& out->nbyte > 0 && fOUT != NULL) {
            fwrite(out->buff, out->nbyte, sizeof(char), fOUT);
        }
    }
    out->obs.data=data;
    out->obs.n=nobs;
}

int main()
{
    //traceopen("rtcm.log");
    tracelevel(4);

    rtcm_t rtcm;// = { 0 };
    if (init_rtcm(&rtcm))
    {
        //FILE* fRTCM_IN = fopen("C:\\rtklib\\2\\sta8100364c34.rtcm3", "rb");
        //FILE* fRTCM_OUT = fopen("C:\\rtklib\\2\\sta8100364c34_.rtcm3", "wb");
        //FILE* fCSV_OUT = fopen("C:\\rtklib\\2\\sta8100364c34.csv", "w");

        FILE* fRTCM_IN = fopen("C:\\rtklib\\jfng\\jfng365i46.rtcm3", "rb");
        FILE* fRTCM_OUT = fopen("C:\\rtklib\\jfng\\jfng365i46_.rtcm3", "wb");
        FILE* fCSV_OUT = fopen("C:\\rtklib\\jfng\\jfng365i46_.csv", "w");

        while (fRTCM_IN != NULL && !feof(fRTCM_IN))
        {
            int ret = input_rtcm3f(&rtcm, fRTCM_IN);
            if (ret == 1)
            {
                int i = 0, j = 0;
                int ng = 0, nr = 0, ne = 0, ns = 0, nj = 0, nc = 0;
                obs_t* obs = &rtcm.obs;
                for (i = 0; i < obs->n; ++i)
                {
                    obsd_t* obsd = obs->data + i;
                    int wn = 0, prn = 0;
                    double ws = time2gpst(obsd->time, &wn);
                    char id[4] = { 0 };
                    satno2id(obsd->sat, id);
                    char sys = satsys(obsd->sat, &prn);
                    if (sys == SYS_GPS)
                        ++ng;
                    else if (sys == SYS_GLO)
                        ++nr;
                    else if (sys == SYS_GAL)
                        ++ne;
                    else if (sys == SYS_SBS)
                        ++ns;
                    else if (sys == SYS_QZS)
                        ++nj;
                    else if (sys == SYS_CMP)
                        ++nc;
                    for (j = 0; j < NFREQ + NEXOBS; ++j)
                    {
                        if (obsd->code[j] == 0) continue;
                        printf("%4i,%10.3f,%s,%2i,%14.4f,%14.4f,%10.4f,%3i\n", wn, ws, id, obsd->code[j], obsd->P[j], obsd->L[j], obsd->D[j], obsd->SNR[j]);
                        if (fCSV_OUT)
                            fprintf(fCSV_OUT, "%4i,%10.3f,%s,%2i,%14.4f,%14.4f,%10.4f,%3i\n", wn, ws, id, obsd->code[j], obsd->P[j], obsd->L[j], obsd->D[j], obsd->SNR[j]);
                    }
                }
                if (ng > 0) write_rtcm3_msm(&rtcm, 1077, (nr + ne + ns + nj + nc) > 0, fRTCM_OUT); /* GPS */
                if (nr > 0) write_rtcm3_msm(&rtcm, 1087, (ne + ns + nj + nc) > 0, fRTCM_OUT);/* GLO */
                if (ne > 0) write_rtcm3_msm(&rtcm, 1097, (ns + nj + nc) > 0, fRTCM_OUT); /* GAL */
                if (ns > 0) write_rtcm3_msm(&rtcm, 1107, (nj + nc) > 0, fRTCM_OUT); /* SBAS */
                if (nj > 0) write_rtcm3_msm(&rtcm, 1117, nc > 0, fRTCM_OUT); /* QZS */
                if (nc > 0) write_rtcm3_msm(&rtcm, 1127, 0, fRTCM_OUT); /* CMP */

#if 1
                /* test with encoder without make cell according to number of satellites, this will only output max 64 nsat*nsig */
                gen_rtcm3(&rtcm, 1127, 0);

                decode_rtcm(&rtcm);
                if (rtcm.nbyte > 0 && fRTCM_OUT != NULL) fwrite(rtcm.buff, rtcm.nbyte, sizeof(char), fRTCM_OUT);
#endif

                memset(rtcm.buff, 0, sizeof(rtcm.buff));
                rtcm.nbyte = rtcm.len = rtcm.nbit = 0;
            }
            else if (ret == 2)
            {
                /* output eph */
                char id[4] = { 0 };
                int prn = 0;
                satno2id(rtcm.ephsat, id);
                char sys = satsys(rtcm.ephsat, &prn);
                if (sys == SYS_GPS)
                {
                    gen_rtcm3(&rtcm, 1019, 0);
                    if (rtcm.nbyte > 0 && fRTCM_OUT != NULL) fwrite(rtcm.buff, rtcm.nbyte, sizeof(char), fRTCM_OUT);
                }
                else if (sys == SYS_GLO)
                {
                    gen_rtcm3(&rtcm, 1020, 0);
                    if (rtcm.nbyte > 0 && fRTCM_OUT != NULL) fwrite(rtcm.buff, rtcm.nbyte, sizeof(char), fRTCM_OUT);
                }
                else if (sys == SYS_GAL)
                {
                    gen_rtcm3(&rtcm, 1045, 0);
                    if (rtcm.nbyte > 0 && fRTCM_OUT != NULL) fwrite(rtcm.buff, rtcm.nbyte, sizeof(char), fRTCM_OUT);
                }
                else if (sys == SYS_SBS)
                {
                    int i = 0;
                }
                else if (sys == SYS_QZS)
                {
                    gen_rtcm3(&rtcm, 1044, 0);
                    if (rtcm.nbyte > 0 && fRTCM_OUT != NULL) fwrite(rtcm.buff, rtcm.nbyte, sizeof(char), fRTCM_OUT);
                }
                else if (sys == SYS_CMP)
                {
                    gen_rtcm3(&rtcm, 1042, 0);
                    if (rtcm.nbyte > 0 && fRTCM_OUT != NULL) fwrite(rtcm.buff, rtcm.nbyte, sizeof(char), fRTCM_OUT);
                }
                int i = 0;
                memset(rtcm.buff, 0, sizeof(rtcm.buff));
                rtcm.nbyte = rtcm.len = rtcm.nbit = 0;
            }
            else if (ret == 5)
            {
                /* output 1005/1006 */
                int i = 0;
                gen_rtcm3(&rtcm, 1005, 0);
                if (rtcm.nbyte > 0 && fRTCM_OUT != NULL) fwrite(rtcm.buff, rtcm.nbyte, sizeof(char), fRTCM_OUT);

                memset(rtcm.buff, 0, sizeof(rtcm.buff));
                rtcm.nbyte = rtcm.len = rtcm.nbit = 0;

            }
        }
        free_rtcm(&rtcm);
        if (fRTCM_IN != NULL) fclose(fRTCM_IN);
        if (fRTCM_OUT != NULL) fclose(fRTCM_OUT);
        if (fCSV_OUT != NULL) fclose(fCSV_OUT);
    }

    //traceclose();
}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
