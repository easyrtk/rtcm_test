// rtcm_test.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "rtklib.h"

int main()
{
    traceopen("rtcm.log");
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
                if (ng > 0) /* GPS */
                {
                    gen_rtcm3(&rtcm, 1077, (nr + ne + ns + nj + nc) > 0);
                    if (rtcm.nbyte > 0 && fRTCM_OUT != NULL) fwrite(rtcm.buff, rtcm.nbyte, sizeof(char), fRTCM_OUT);
                }
                if (nr > 0) /* GLO */
                {
                    gen_rtcm3(&rtcm, 1087, (ne + ns + nj + nc) > 0);
                    if (rtcm.nbyte > 0 && fRTCM_OUT != NULL) fwrite(rtcm.buff, rtcm.nbyte, sizeof(char), fRTCM_OUT);
                }
                if (ne > 0) /* GAL */
                {
                    gen_rtcm3(&rtcm, 1097, (ns + nj + nc) > 0);
                    if (rtcm.nbyte > 0 && fRTCM_OUT != NULL) fwrite(rtcm.buff, rtcm.nbyte, sizeof(char), fRTCM_OUT);
                }
                if (ns > 0) /* SBAS */
                {
                    gen_rtcm3(&rtcm, 1107, (nj + nc) > 0);
                    if (rtcm.nbyte > 0 && fRTCM_OUT != NULL) fwrite(rtcm.buff, rtcm.nbyte, sizeof(char), fRTCM_OUT);
                }
                if (nj > 0) /* QZS */
                {
                    gen_rtcm3(&rtcm, 1117, nc > 0);
                    if (rtcm.nbyte > 0 && fRTCM_OUT != NULL) fwrite(rtcm.buff, rtcm.nbyte, sizeof(char), fRTCM_OUT);
                }
                if (nc > 0) /* CMP */
                {
                    gen_rtcm3(&rtcm, 1127, 0);
                    if (rtcm.nbyte > 0 && fRTCM_OUT != NULL) fwrite(rtcm.buff, rtcm.nbyte, sizeof(char), fRTCM_OUT);
                }
                memset(rtcm.buff, 0, sizeof(rtcm.buff));
                rtcm.nbyte = rtcm.len = rtcm.nbit = 0;
            }
        }
        free_rtcm(&rtcm);
        if (fRTCM_IN != NULL) fclose(fRTCM_IN);
        if (fRTCM_OUT != NULL) fclose(fRTCM_OUT);
        if (fCSV_OUT != NULL) fclose(fCSV_OUT);
    }

    traceclose();
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
