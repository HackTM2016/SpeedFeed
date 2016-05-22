#include "RestSender.h"

#pragma comment(lib, "cpprest110_1_1")

#include <iostream>

using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace pplx;

CRESTSender::CRESTSender(web::http::client::http_client & client)
				: m_httpClient(client)
{
}

CRESTSender::~CRESTSender(void)
{
}

task<http_response> CRESTSender::task_request(web::http::method mtd, 
												web::json::value const & jvalue)
{
   return (methods::POST == mtd || methods::HEAD == mtd) 
				? m_httpClient.request(mtd, L"", jvalue) 
				: m_httpClient.request(mtd, L"");
}

task<http_response> CRESTSender::task_string_request(web::http::method mtd, 
														const utility::string_t &body_data)
{
	return(methods::POST == mtd || methods::HEAD == mtd) 
				? m_httpClient.request(mtd, L"", body_data, _XPLATSTR("application/json")) 
				: m_httpClient.request(mtd, L"");
}
 
void CRESTSender::make_request(web::http::method mtd, web::json::value const & jvalue)
{
   task_request(mtd, jvalue)
      .then([](http_response response)
      { 
	     if (status_codes::OK == response.status_code())
         {
            return response.extract_json();
         }

         return pplx::task_from_result(json::value());
      })
      .wait();
}
