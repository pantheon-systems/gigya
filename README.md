## Patch for Gigya for Drupal 7 and Drupal 8

[![Unofficial](https://img.shields.io/badge/Pantheon-Unofficial-yellow?logo=pantheon&color=FFDC28)](https://pantheon.io/docs/oss-support-levels#unofficial)

This is a patched gigya for drupal 7 and drupal 8. Since pantheon has a limitation for reading $_COOKIE via PHP. 

The GConnector is a module for Drupal 7 and 8 it allows you to easily integrate Gigya's Customer Identity in your Drupal site. With the GConnector, you can easily implement such features as registration, authentication, profile management, data analytics and third-party integrations. Increase registration rates and identify customers across devices, consolidate data into rich customer profiles, and provide better service, products and experiences by integrating data into marketing and service applications. 

Gigya has two types of session dynamic and fixed. When using dynamic session based the gigya connector is not working, that patch fixes that issue. 


### How to setup gigya on drupal 7 
 - https://developers.gigya.com/display/GD/Drupal+7

[![Screenshot](http://dev-mapinas.pantheonsite.io/sites/default/files/gigya.png)](https://developers.gigya.com/display/GD/Drupal+7)


### How to setup gigya on drupal 8  
 - https://developers.gigya.com/display/GD/Drupal+8

[![Screenshot](http://dev-mapinas.pantheonsite.io/sites/default/files/gigya2.png)](https://developers.gigya.com/display/GD/Drupal+8)


## FAQ

### How to troubleshoot when glt_xxxx cookie is not being generated

1. Open your browser chrome browser's developers tools check for cookies

2. Make sure that when you browse the site you are not logged in

3. When you login, check the generated cookies from your developer tools if glt_xxxxx is not on the list theres something wrong on the side of gigya, might be an incorrect credentials or gigya configuration issue, this is a non-platform issue its on gigya. This can be confirmed by when glt_xxxx is empty.

[![Screenshot](http://dev-mapinas.pantheonsite.io/sites/default/files/cookie2.png)](https://developers.gigya.com/display/GD/Drupal+8)


### How to troubleshoot when gltexp_xxxx cookie is not being generated

1. Open your browser chrome browser's developers tools check for cookies

2. Make sure that when you browse the site you are not logged in

3. If gigya RaaS session was set to dynamic based expect that gltexp_xxxx cookie should be generated if not then make sure that this patch is applied




