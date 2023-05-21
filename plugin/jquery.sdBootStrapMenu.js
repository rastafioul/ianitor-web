/*!
jquery.sdBootStrapMenu.js
Based on
 jQuery.sdScrollMenu v0.2
 (c) 2015 Steve David <http://www.steve-david.com>

 MIT-style license.
 */



    $.lis = [];

 function buildMenu (el, options) {
    var $el = $(el)
        , level
        , css = {
            // 'padding-left': '15px',
            // 'overflow': 'hidden'
        }
        // , $menu = $('<div />').addClass('sdsm-menu')
        , $menu = $("#sdScrollMenu")
        , $h1, $h2, $h3, $h4, $h5, $h6, $li
        , text;

    $menu.css({
        // width: options.width + 'px',
        // position: 'fixed',
        // top: '10px',
        // left: '10px',
        // borderRadius: 5,
        // border: '1px solid #ccc',
        // backgroundColor: '#eee',
        // padding: 10
    });



    $el.find(options.titles).each(function(i) {
        $.lis[i] = $(this);
        level = this.localName;
        text = $(this).text();
        // <ul class="nav flex-column">
        // <li class="nav-item">
        //   <a class="nav-link active" aria-current="page" href="#">Active</a>
        // </li>
        switch(level) {
            case 'h1':
                $h1 = $('<ul />');//.css(css);
                $h1.addClass("nav flex-column");
                let thisLi = $h1.append($('<li class="nav-item" />').html('<a class="nav-link active" href="#'+$(this).prop("id")+'">'+text+'</a> ').attr({'data-id': i}));
                console.log("h1: "+text+":"+JSON.stringify($h1));
                $menu.append($h1);
                break;
            case 'h2':
                $h2 = $('<ul />');//.css(css);
                $li = $h2.append($('<li class="nav-item" />').html('<a class="nav-link active" href="#'+$(this).prop("id")+'">'+text+'</a> ').attr({'data-id': i}));
                // $li.addClass("nav-item");
                // $a = $li.append($('<a />').html(text).attr("href", $(this).prop("id")));
                // $a.addClass("nav-item");
                if(!$h1){
                     $menu.append($li);
                     console.log("h2: appending to menu : "+text+":"+JSON.stringify($h2));

                }
                else{

                    $h1.append($li);
                    console.log("h2: appending to h1 : "+text+":"+JSON.stringify($h2));

                }

                break;
            case 'h3':
                $h3 = $('<ul />');//.css(css);
                $li = $h3.append($('<li />').html('<a class="nav-link active" aria-current="page" href="#'+$(this).prop("id")+'">'+text+'</a> ').attr({'data-id': i}));
                if(!$h2) {$menu.append($li);
                    console.log("h3: appending to menu: "+text+":"+JSON.stringify($h3));
                
                }
                else 
                {
                    $h2.append($li);
                    console.log("h3: appending to h2: "+text+":"+JSON.stringify($h3));

                }

                break;
            case 'h4':
                $h4 = $('<ul />');//.css(css);
                $li = $h4.append($('<li />').html('<a class="nav-link active" aria-current="page" href="#'+$(this).prop("id")+'">'+text+'</a> ').attr({'data-id': i}));
                if(!$h3) 
                {
                    console.log("h4: appending to li: "+text+":"+JSON.stringify($h4));
                    $menu.append($li);}
                else {
                    console.log("h4: appending to h3: "+text+":"+JSON.stringify($h4));
                    $h3.append($li);}

                break;
            case 'h5':
                $h5 = $('<ul />');//.css(css);
                $li = $h5.append($('<li />').html('<a class="nav-link active" aria-current="page" href="#'+$(this).prop("id")+'">'+text+'</a> ').attr({'data-id': i}));
                if(!$h4) $menu.append($li);
                else $h4.append($li);
                console.log("h5: "+text+":"+JSON.stringify($h5));

                break;
            case 'h6':
                $h6 = $('<ul />');//.css(css);
                $li = $h6.append($('<li />').html('<a class="nav-link active" aria-current="page" href="#'+$(this).prop("id")+'">'+text+'</a> ').attr({'data-id': i}));
                if(!$h5) $menu.append($li);
                else $h5.append($li);
                console.log("h6: "+text+":"+JSON.stringify($h6));

                break;
        }
    });

    //$('body').prepend($menu);

    return $menu;
};

//  ;(function($) {

//     $.fn.extend({
//         sdScrollMenu: function(options) {
//             if (options && typeof(options) == 'object') {
//                 options = $.extend({}, $.sdScrollMenu.defaults, options);
//             }

//             if($(this).length == 1) {
//                 new $.sdScrollMenu(this, options);
//             }

//             return this;
//         }
//     });


//     $.lis = [];
//     $.sdScrollMenu.buildMenu = function(el, options) {
//         var $el = $(el)
//             , level
//             , css = {
//                 'padding-left': '15px',
//                 'overflow': 'hidden'
//             }
//             , $menu = $('<div />').addClass('sdsm-menu')
//             , $h1, $h2, $h3, $h4, $h5, $h6, $li
//             , text;

//         $menu.css({
//             width: options.width + 'px',
//             position: 'fixed',
//             top: '10px',
//             left: '10px',
//             borderRadius: 5,
//             border: '1px solid #ccc',
//             backgroundColor: '#eee',
//             padding: 10
//         });



//         $el.find(options.titles).each(function(i) {
//             $.lis[i] = $(this);
//             level = this.localName;
//             text = $(this).text();

//             switch(level) {
//                 case 'h1':
//                     $h1 = $('<ul />');//.css(css);
//                     $h1.append($('<li />').html(text).attr({'data-id': i}));
//                     break;
//                 case 'h2':
//                     $h2 = $('<ul />');//.css(css);
//                     $li = $h2.append($('<li />').html(text).attr({'data-id': i}));
//                     $li.addClass("nav-item");
//                     $a = $li.append($('<a />').html(text).attr("href", $(this).prop("id")));
//                     $a.addClass("nav-item");
//                     if(!$h1) $menu.append($li);
//                     else $h1.append($li);
//                     break;
//                 case 'h3':
//                     $h3 = $('<ul />');//.css(css);
//                     $li = $h3.append($('<li />').html(text).attr({'data-id': i}));
//                     if(!$h2) $menu.append($li);
//                     else $h2.append($li);
//                     break;
//                 case 'h4':
//                     $h4 = $('<ul />');//.css(css);
//                     $li = $h4.append($('<li />').html(text).attr({'data-id': i}));
//                     if(!$h3) $menu.append($li);
//                     else $h3.append($li);
//                     break;
//                 case 'h5':
//                     $h5 = $('<ul />');//.css(css);
//                     $li = $h5.append($('<li />').html(text).attr({'data-id': i}));
//                     if(!$h4) $menu.append($li);
//                     else $h4.append($li);
//                     break;
//                 case 'h6':
//                     $h6 = $('<ul />');//.css(css);
//                     $li = $h6.append($('<li />').html(text).attr({'data-id': i}));
//                     if(!$h5) $menu.append($li);
//                     else $h5.append($li);
//                     break;
//             }
//         });

//         $('body').prepend($menu);

//         return $menu;
//     };

//     $.sdScrollMenu.scrollMenu = function($li, $menu, options) {
//         if(typeof($li.position()) !== 'undefined') {
//             var liTop = $li.position().top;
//             if(liTop > $menu.height() || liTop < 0) {
//                 $menu.animate({
//                     scrollTop: liTop
//                 }, {
//                     duration: 500,
//                 });
//             }
//         }
//     };  

//     $.sdScrollMenu.innerScroll = function($menu, options) {
//         if($menu.css('overflow-y') == 'scroll') {
//             $menu.css({
//                 'height': 'auto',
//                 'overflow-y': 'auto',
//             })
//         } else if($menu.height() > $(window).height()) {
//             $menu.css({
//                 'height': $(window).height() - $menu.position().top,
//                 'overflow-y': 'scroll',
//             });

            
//         }


//     };

//     $.sdScrollMenu.activeLinks = function(that, $menu, options) {
//         var positionTop = $(that).scrollTop()
//             , interval = positionTop + $(window).height()
//             , offsetTop
//             , k = 0
//             , activeLis = [];      

//         $($.lis).each(function(i) {
//             offsetTop = $(this).offset().top;
//             if(offsetTop <= interval && offsetTop >= positionTop) {
//                 ++k;
//                 activeLis[i] = $menu.find('[data-id=' + i + ']');
//             }
//         });

//         if(!k) {
//             $menu.find('li.active:not(:last-child)').removeClass('active');

//         } else {
//             $menu.find('li.active').removeClass('active');
//         }

//         for(var i = 0; i < activeLis.length; ++i) {
//             $(activeLis[i]).addClass('active');
//         }



//         $menu.find('.active').removeClass('first').eq(0).addClass('first');

//         $.sdScrollMenu.scrollMenu($menu.find('.active.first'), $menu, options);
//     };

//     $.sdScrollMenu.highlightTitle = function($h, options) {
//         $h.css({'background-color': options.highlightColor});
//         $h
//             .delay(500)
//             .css({
//                 '-webkit-transition': 'all '+ options.highlightDuration +'ms ease',
//                 '-moz-transition': 'all '+ options.highlightDuration +'ms ease',
//                 '-ms-transition': 'all '+ options.highlightDuration +'ms ease',
//                 '-o-transition': 'all '+ options.highlightDuration +'ms ease',
//                 'transition': 'all '+ options.highlightDuration +'ms ease'
//             });

//         window.setTimeout(function() {
//             $h.css('background-color', 'transparent');
//         }, 500);

//     };

//     $.sdScrollMenu.scrollTo = function(el, index, options) {
//         var $el = $(el)
//             , $title = $el.find(options.titles).eq(index)
//             , scrollTo = $title.offset().top;

//         $('body').animate({
//             scrollTop: scrollTo
//         }, {
//             duration: options.scrollDuration,
//             complete: function() {
//                 $.sdScrollMenu.highlightTitle($title, options)
//             }
//         })
//     };



//     $.sdScrollMenu.defaults = {
//         width: 150,
//         scrollDuration: 500,
//         titles: 'h1, h2, h3, h4, h5, h6',
//         highlightDuration: 1000,
//         highlightColor: '#146'
//     };

//     $.sdScrollMenu = function(el, options) {
//         var $el = $(el)
//             , $menu;

//         $menu = $.sdScrollMenu.buildMenu(el, options);

//         // $menu.find('li').css('cursor', 'pointer').on('click', function(e) {
//         //     $.sdScrollMenu.scrollTo(el, $(this).attr('data-id'), options);
//         // });

//         // $(window).on('scroll', function() {
//         //     $.sdScrollMenu.activeLinks(this, $menu, options);
//         // });

//         // $(window).on('load', function() {
//         //     $.sdScrollMenu.activeLinks(this, $menu, options);
//         //     $.sdScrollMenu.innerScroll($menu, options);
//         // });

//         // $(window).on('resize', function() {
//         //     $.sdScrollMenu.activeLinks(this, $menu, options);
//         //     $.sdScrollMenu.innerScroll($menu, options);
//         // });

//     };
    
 
    

// })(jQuery);
