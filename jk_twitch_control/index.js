const rosnodejs = require('rosnodejs');
const geometry_msgs = rosnodejs.require('geometry_msgs').msg;
const fs = require('fs');
const tmi = require('tmi.js');

const Aggregator = require('./aggregator.js');

let config = fs.readFileSync(__dirname + '/config.json');
config = JSON.parse(config);

const client = new tmi.client({
	identity: {
		username: config.username,
		password: config.password
	},
	channels: [config.channel]
});



rosnodejs.initNode('/jk_twitch_node').then(function (rosNode) {
	let pub = rosNode.advertise('/jk/twitch_cmd', geometry_msgs.Twist);
	
	let jk_mode = rosNode.subscribe('/jk/mode', 'std_msgs/String', function (data) {
		client.say(config.channel, "MODE CHANGED: " + data.data);
	});

	function onMessageHandler (target, context, msg, self) {
		if (self) { return; }
		const cmd = msg.trim();
		aggregator.parse(cmd);
	}
	
	function onTick(vote) {
		const msg = new geometry_msgs.Twist();
		msg.linear.x = vote.vector.x;
		msg.linear.y = vote.vector.y;
		
		if (msg.linear.x == 0 && msg.linear.y == 0) {
			return;
		}
		
		pub.publish(msg);
	}
	
	let aggregator = new Aggregator();
	aggregator.on("tick", onTick);
	aggregator.freq = 1.5;

	client.on('message', onMessageHandler);
	client.connect();
	aggregator.run();

	console.log("Connected to Twitch channel: " + config.channel);
});
