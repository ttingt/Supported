/**
 * Welcome to Pebble.js!
 *
 * This is where you write your app.
 */

var UI = require('ui');
var Vector2 = require('vector2');
var Vibe = require('ui/vibe');
var Check = new UI.Image({
  position: new Vector2(10, 10),
  size: new Vector2(10, 10),
  image: 'checkmark10.png'
});

var timeText = new UI.TimeText({
  position: new Vector2(0, 80),
  size: new Vector2(144, 84),
  text: "%H:%M",
  font: 'bitham-42-bold',
  color: 'white',
  textAlign: 'center'
});

var rect = new UI.Rect({
  position: new Vector2(0, 80),
  size: new Vector2(144, 1),
  backgroundColor: 'white'
});

var dateText = new UI.TimeText({
  position: new Vector2(4, 50),
  size: new Vector2(144, 30),
  text: "%B %d",
  font: 'gothic-24-bold',
  color: 'white',
  textAlign: 'left'
});


var main = new UI.Window({
  
//   title: timeText,
//   icon: 'images/menu_icon.png',
//   subtitle: 'Hello World!',
//   body: 'Press any button.'
   
});

main.add(timeText);
main.add(dateText);
main.add(rect);
main.add(Check);

main.show();
Vibe.vibrate('long');

main.on('click', 'up', function(e) {
  var menu = new UI.Menu({
    sections: [{
      items: [{
        title: 'Pebble.js',
        icon: 'images/menu_icon.png',
        subtitle: 'Can do Menus'
      }, {
        title: 'Second Item',
        subtitle: 'Subtitle Text'
      }]
    }]
  });
  menu.on('select', function(e) {
    console.log('Selected item #' + e.itemIndex + ' of section #' + e.sectionIndex);
    console.log('The item is titled "' + e.item.title + '"');
  });
  menu.show();
});

main.on('click', 'select', function(e) {
  var wind = new UI.Window({
    fullscreen: true,
  });
  var textfield = new UI.Text({
    position: new Vector2(0, 65),
    size: new Vector2(144, 30),
    font: 'gothic-24-bold',
    text: 'Text Anywhere!',
    textAlign: 'center'
  });
  wind.add(textfield);
  wind.show();
});

main.on('click', 'down', function(e) {
  var card = new UI.Card();
//   card.title('A Card');
  card.subtitle('12:12');
  card.body('June 27');
  card.show();
});
