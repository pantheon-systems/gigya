/**
 * @file
 * Gigya functions from Drupal
 */
/*global Drupal, sendSetSSOToken, gigya */

/**
 * For determining the full path to Drupal, useful for when Drupal is installed in a subdirectory of the main website.
 *
 * @property Drupal.settings.pathPrefix
 * @property Drupal.settings.basePath
 */
var getUrlPrefix = function () {
    return Drupal.settings.pathPrefix ? Drupal.settings.basePath + Drupal.settings.pathPrefix : Drupal.settings.basePath;
};

(function ($) {
    /**
     * @todo Undocumented Code!
     */
    $.extend({
        getUrlVars: function () {
            var vars = [], hash;
            var hashes = window.location.href.slice(window.location.href.indexOf('?') + 1).split('&');
            for (var i = 0; i < hashes.length; i++) {
                hash = hashes[i].split('=');
                vars.push(hash[0]);
                vars[hash[0]] = hash[1];
            }
            return vars;
        },
        getUrlVar: function (name) {
            return $.getUrlVars()[name];
        }
    });

    Drupal.gigya = Drupal.gigya || {};

    Drupal.gigya.hideLogin = function () {
        $('#user-login').hide();
    };

    /**
     * @property Drupal.settings.gigya.logoutLocation
     */
    Drupal.gigya.logoutResponse = function (response) {
        if (response['status'] === 'OK') {
            document.getElementById('logoutMessage').innerHTML = "Successfully logged out, redirecting";
            window.location = Drupal.settings.gigya.logoutLocation;
        }
    };

	/**
	 * @param email
	 *
	 * @property Drupal.gigya.toPost.user
	 */
	Drupal.gigya.addEmail = function (email) {
        if (typeof Drupal.gigya.toPost !== 'undefined') {
            Drupal.gigya.toPost.user.email = email;
            Drupal.gigya.login(Drupal.gigya.toPost);
        }
    };

	/**
	 * @param post
	 *
	 * @property ajax.element
	 */
    Drupal.gigya.login = function (post) {
    	var base;
        if (typeof post.context !== 'undefined') {
            base = post.context.id
        }
        else {
            base = $('.gigya-social-login').eq(0).attr('id');
        }
        var element_settings = {};
        element_settings.url = getUrlPrefix() + 'socialize-login';
        element_settings.event = 'gigyaLogin';
        var ajax = new Drupal.ajax(base, $('#' + base), element_settings);
        ajax.options.data = post;
        $(ajax.element).trigger('gigyaLogin').unbind('gigyaLogin');
    };

	/**
	 * Processes onLogout event
	 *
	 * @param response
	 * @returns {boolean}
	 *
	 * @property Drupal.CTools.Modal
	 * @property gigya.socialize
	 * @property response.provider
	 * @property response.user.isSiteUID
	 */
    Drupal.gigya.loginCallback = function (response) {
        Drupal.gigya.toPost = response;

        if ((response.user.email.length === 0) && (response.user.isSiteUID !== true)) {
            var html = '<div class="form-item form-type-textfield form-item-email"><div class="description">Additional information is required in order to complete your registeration. Please fill-in the following info:</div><label for="email" style="float: none;">Email <span class="form-required" title="This field is required.">*</span></label><input type="text" id="email" name="email" value="" size="20" maxlength="60" class="form-text required"><button type="button" class="button" onClick="Drupal.gigya.addEmail(jQuery(this).prev().val())">' + Drupal.t('Submit') + '</button></div>';
            Drupal.CTools.Modal.show();
            $('#modal-title').html('Please fill-in missing details');
            $('#modal-content').html(html);
            $('#modalContent .close').on('click', function () {
                gigya.socialize.logout();
            });
            return false;
        }

        if (response.provider === 'site') {
            return false;
        }

        Drupal.gigya.login(response);
    };

	/**
	 * Send the account object that is returned in the onLogin event to Drupal.
	 *
	 * @param data
	 */
    Drupal.gigya.raasRegLogin = function (data) {
        $('body').append('<div id="gigyaRequestFormsExt"></div>');
        var base = 'gigyaRequestFormsExt',
            element_settings = {},
            id = "#" + base;
        element_settings.url = getUrlPrefix() + 'raas-login';
        element_settings.event = 'gigyaLogin';
        var ajax = new Drupal.ajax(base, $(id), element_settings);
        ajax.options.data = data;
        $(id).trigger('gigyaLogin').unbind('gigyaLogin');
    };

    Drupal.gigya.profileUpdated = function (data) {
        if (data.response.errorCode === 0) {
            var base = 'gigyaRequestForms',
                element_settings = {},
                id = "#" + base,
                gigyaData = {
                    "UID": data.response.UID,
                    "UIDSignature": data.response.UIDSignature,
                    "signatureTimestamp": data.response.signatureTimestamp
                };
            element_settings.url = getUrlPrefix() + 'raas-profile-update';
            element_settings.event = 'gigyaProfileUp';
            var ajax = new Drupal.ajax(base, $(id), element_settings);
            ajax.options.data = {"gigyaData": gigyaData};
            $(id).trigger('gigyaProfileUp').unbind('gigyaProfileUp');
        }
    };

    /**
     * Callback for the getUserInfo function.
     *
     * Takes the getUserInfo object and renders the HTML to display an array
     * of the user object
     *
     * TODO: probably should be removed in production, since its just for dumping
     * user output.
     */
    Drupal.gigya.getUserInfoCallback = function (response) {
        if (response.status === 'OK') {
            var user = response['user'];
            var str = "<pre>"; /* Variable which will hold property values */
            for (var prop in user) {
                if (prop === 'birthYear' && user[prop] == 2009) {
                    user[prop] = '';
                }
                if (prop === 'identities') {
                    for (var ident in user[prop]) {
                        for (var provide in user[prop][ident]) {
                            str += provide + " SUBvalue :" + user[prop][ident][provide] + "\n";
                        }
                    }
                }

                /* Concat prop and its value from object. */
                str += prop + " value :" + user[prop] + "\n";
            }
            str += "</pre>";

            document.getElementById('userinfo').innerHTML = str;
        }
    };

	/**
	 * @param connectUIParams
	 */
	Drupal.gigya.showAddConnectionsUI = function (connectUIParams) {
        gigya.services.socialize.showAddConnectionsUI(connectUIParams);
    };

	/**
	 * @param response
	 */
	Drupal.gigya.notifyLoginCallback = function (response) {
        if (response['status'] === 'OK') {
            setTimeout("$.get(getUrlPrefix() + 'socialize-ajax/notify-login')", 1000);
        }
    };

    /**
     * @todo Undocumented Code!
     */
    Drupal.gigya.initResponse = function (response) {
        if (null != response.user) {
            if (response.user.UID !== Drupal.settings.gigya.notifyLoginParams.siteUID || !response.user.isLoggedIn) {
                gigya.services.socialize.notifyLogin(Drupal.settings.gigya.notifyLoginParams);
            }
        }
    };

    /**
     * This function checks if the user is logged in at Gigya if so it renders the Gamification Plugin
     */
    Drupal.gigya.gmInit = function (response) {
        if (response.errorCode === 0) {
            if (typeof response.UID !== 'undefined') {
                Drupal.gigya.gmRender();
            }
        }
        return false;
    };

    /**
     * function that renders the Gamification Plugin
     */
    Drupal.gigya.gmRender = function () {
        $.each(Drupal.settings.gigyaGM, function (key, block) {
            $.each(block, function (name, params) {
                switch (name) {
                    case 'challengeStatusParams':
                        gigya.gm.showChallengeStatusUI(params);
                        break;
                    case 'userStatusParams':
                        gigya.gm.showUserStatusUI(params);
                        break;
                    case 'achievementsParams':
                        gigya.gm.showAchievementsUI(params);
                        break;
                    case 'leaderboardParams':
                        gigya.gm.showLeaderboardUI(params);
                        break;
                }
            });
        });
    };
    /**
     * This function checks if the user is looged in at gigya if so it renders the Gamification notification Plugin
	 *
	 * @property Drupal.settings.gigyaGMnotification
	 * @property gigya.gm
     */
    Drupal.gigya.gmNotiInit = function (response) {
        if (response.errorCode === 0) {
            if (typeof response.UID !== 'undefined') {
                var params = Drupal.settings.gigyaGMnotification;
                delete params.enable;
                gigya.gm.showNotifications(params);
            }
        }
        return false;
    };

    Drupal.gigya.logoutCallback = function (res) {
    	/* Some kind of page refresh is necessary for SSO where logout is passive. When logout is active through Drupal, a refresh is unnecessary. */
		window.location.reload();
    };

    /**
     * AJAX command to place HTML within a BeautyTip.
	 *
	 * @property response.redirectTarget
     **/
    Drupal.gigya.gigyaLoginRedirect = function (ajax, response, status) {
        if (typeof sendSetSSOToken === 'undefined' || sendSetSSOToken === false) {
            if (response.redirectTarget) {
                location.replace(response.redirectTarget);
            }
            else {
                location.reload();
            }
        }
        else if (sendSetSSOToken === true) {
            gigya.setSSOToken({redirectURL: response.redirectTarget ? response.redirectTarget : '/'});
        }
    };

	/**
	 * @property Drupal.ajax.prototype.commands
	 */
	$(function () {
        Drupal.ajax.prototype.commands.gigyaLoginRedirect = Drupal.gigya.gigyaLoginRedirect;
    });
})(jQuery);