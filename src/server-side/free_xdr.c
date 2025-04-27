//
// Created by hector-pc on 27/04/25.
//
#include "claves_rpc.h"
#include <rpc/rpc.h>

bool_t
claves_prog_1_freeresult(SVCXPRT    *transp,
                        xdrproc_t   xdr_res,
                        caddr_t     resultp)
{
    (void)transp;
    xdr_free(xdr_res, resultp);
    return TRUE;
}