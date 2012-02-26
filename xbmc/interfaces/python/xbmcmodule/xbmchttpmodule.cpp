/*
 *      Copyright (C) 2011 Team XBMC
 *      http://www.xbmc.org
 *
 *  This Program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2, or (at your option)
 *  any later version.
 *
 *  This Program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with XBMC; see the file COPYING.  If not, write to
 *  the Free Software Foundation, 675 Mass Ave, Cambridge, MA 02139, USA.
 *  http://www.gnu.org/copyleft/gpl.html
 *
 */

#include <Python.h>

#include "HttpRequest.h"
#include "multidict.h"

#include "pyutil.h"
#include "pythreadstate.h"

using namespace std;
using namespace PYXBMC;


#if defined(__GNUG__) && (__GNUC__>4) || (__GNUC__==4 && __GNUC_MINOR__>=2)
#pragma GCC diagnostic ignored "-Wstrict-aliasing"
#endif

#ifdef __cplusplus
extern "C" {
#endif

  namespace xbmchttp
  {
    /*****************************************************************
     * start of xbmchttp methods
     *****************************************************************/

    // define c functions to be used in python here
    PyMethodDef xbmcHttpMethods[] = {
      {NULL, NULL, 0, NULL}
    };

    /*****************************************************************
     * end of methods and python objects
     * initxbmc(void);
     *****************************************************************/

    PyMODINIT_FUNC
    InitHttpTypes(void)
    {
      initHttpRequest_Type();
      initmultidict_Type();
      initmultidictiter_Type();

      if (PyType_Ready(&HttpRequest_Type) < 0 ||
          PyType_Ready(&multidict_Type) < 0 ||
          PyType_Ready(&multidictiter_Type) < 0)
        return;
    }

    PyMODINIT_FUNC
    DeinitHttpModule()
    {
      // no need to Py_DECREF our objects (see InitXBMCMHttpModule()) as they were created only
      // so that they could be added to the module, which steals a reference.
    }

    PyMODINIT_FUNC
    InitHttpModule()
    {
      Py_INCREF(&HttpRequest_Type);
      Py_INCREF(&multidict_Type);
      Py_INCREF(&multidictiter_Type);

      // init general xbmc modules
      PyObject* pXbmcHttpModule;
      pXbmcHttpModule = Py_InitModule((char*)"xbmchttp", xbmcHttpMethods);
      if (pXbmcHttpModule == NULL) return;

      // Add request object
      PyModule_AddObject(pXbmcHttpModule, (char*)"HttpRequest", (PyObject*)&HttpRequest_Type);
      PyModule_AddObject(pXbmcHttpModule, (char*)"multidict", (PyObject*)&multidict_Type);
      PyModule_AddObject(pXbmcHttpModule, (char*)"multidictiter", (PyObject*)&multidictiter_Type);

      // Add HTTP method constants
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"GET", (char*)"GET");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"POST", (char*)"POST");

      // Add HTTP status code constants
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_CONTINUE", 100);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_SWITCHING_PROTOCOLS", 101);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_PROCESSING", 102);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_OK", 200);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_CREATED", 201);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_ACCEPTED", 202);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_NON_AUTHORITATIVE_INFORMATION", 203);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_NO_CONTENT", 204);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_RESET_CONTENT", 205);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_PARTIAL_CONTENT", 206);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_MULTI_STATUS", 207);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_MULTIPLE_CHOICES", 300);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_MOVED_PERMANENTLY", 301);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_FOUND", 302);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_SEE_OTHER", 303);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_NOT_MODIFIED", 304);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_USE_PROXY", 305);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_SWITCH_PROXY", 306);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_TEMPORARY_REDIRECT", 307);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_BAD_REQUEST", 400);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_UNAUTHORIZED", 401);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_PAYMENT_REQUIRED", 402);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_FORBIDDEN", 403);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_NOT_FOUND", 404);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_METHOD_NOT_ALLOWED", 405);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_METHOD_NOT_ACCEPTABLE", 406);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_PROXY_AUTHENTICATION_REQUIRED", 407);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_REQUEST_TIMEOUT", 408);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_CONFLICT", 409);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_GONE", 410);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_LENGTH_REQUIRED", 411);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_PRECONDITION_FAILED", 412);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_REQUEST_ENTITY_TOO_LARGE", 413);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_REQUEST_URI_TOO_LONG", 414);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_UNSUPPORTED_MEDIA_TYPE", 415);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_REQUESTED_RANGE_NOT_SATISFIABLE", 416);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_EXPECTATION_FAILED", 417);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_UNPROCESSABLE_ENTITY", 422);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_LOCKED", 423);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_FAILED_DEPENDENCY", 424);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_UNORDERED_COLLECTION", 425);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_UPGRADE_REQUIRED", 426);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_RETRY_WITH", 449);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_INTERNAL_SERVER_ERROR", 500);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_NOT_IMPLEMENTED", 501);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_BAD_GATEWAY", 502);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_SERVICE_UNAVAILABLE", 503);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_GATEWAY_TIMEOUT", 504);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_HTTP_VERSION_NOT_SUPPORTED", 505);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_VARIANT_ALSO_NEGOTIATES", 506);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_INSUFFICIENT_STORAGE", 507);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_BANDWIDTH_LIMIT_EXCEEDED", 509);
      PyModule_AddIntConstant(pXbmcHttpModule, (char*)"STATUS_NOT_EXTENDED", 510);

      // Add HTTP header fields
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_ACCEPT", (char*)"Accept");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_ACCEPT_CHARSET", (char*)"Accept-Charset");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_ACCEPT_ENCODING", (char*)"Accept-Encoding");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_ACCEPT_LANGUAGE", (char*)"Accept-Language");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_ACCEPT_RANGES", (char*)"Accept-Ranges");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_AGE", (char*)"Age");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_ALLOW", (char*)"Allow");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_AUTHORIZATION", (char*)"Authorization");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_CACHE_CONTROL", (char*)"Cache-Control");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_CONNECTION", (char*)"Connection");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_CONTENT_ENCODING", (char*)"Content-Encoding");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_CONTENT_LANGUAGE", (char*)"Content-Language");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_CONTENT_LENGTH", (char*)"Content-Length");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_CONTENT_LOCATION", (char*)"Content-Location");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_CONTENT_MD5", (char*)"Content-MD5");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_CONTENT_RANGE", (char*)"Content-Range");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_CONTENT_TYPE", (char*)"Content-Type");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_COOKIE", (char*)"Cookie");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_DATE", (char*)"Date");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_ETAG", (char*)"ETag");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_EXPECT", (char*)"Expect");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_EXPIRES", (char*)"Expires");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_FROM", (char*)"From");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_HOST", (char*)"Host");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_IF_MATCH", (char*)"If-Match");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_IF_MODIFIED_SINCE", (char*)"If-Modified-Since");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_IF_NONE_MATCH", (char*)"If-None-Match");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_IF_RANGE", (char*)"If-Range");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_IF_UNMODIFIED_SINCE", (char*)"If-Unmodified-Since");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_LAST_MODIFIED", (char*)"Last-Modified");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_LOCATION", (char*)"Location");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_MAX_FORWARDS", (char*)"Max-Forwards");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_PRAGMA", (char*)"Pragma");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_PROXY_AUTHENTICATE", (char*)"Proxy-Authenticate");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_PROXY_AUTHORIZATION", (char*)"Proxy-Authorization");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_RANGE", (char*)"Range");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_REFERER", (char*)"Referer");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_RETRY_AFTER", (char*)"Retry-After");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_SERVER", (char*)"Server");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_SET_COOKIE", (char*)"Set-Cookie");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_SET_COOKIE2", (char*)"Set-Cookie2");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_TE", (char*)"TE");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_TRAILER", (char*)"Trailer");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_TRANSFER_ENCODING", (char*)"Transfer-Encoding");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_UPGRADE", (char*)"Upgrade");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_USER_AGENT", (char*)"User-Agent");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_VARY", (char*)"Vary");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_VIA", (char*)"Via");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_WARNING", (char*)"Warning");
      PyModule_AddStringConstant(pXbmcHttpModule, (char*)"HEADER_WWW_AUTHENTICATE", (char*)"WWW-Authenticate");
    }
  }

#ifdef __cplusplus
}
#endif
