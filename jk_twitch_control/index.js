const rosnodejs = require('rosnodejs');
const geometry_msgs = rosnodejs.require('geometry_msgs').msg;
const fs = require('fs');
const tmi = require('tmi.js');

const Aggregator = require('./aggregator.js');


let config = fs.readFileSync('./config.json');
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

	function onMessageHandler (target, context, msg, self) {
		if (self) { return; }
		// client.say(target, "Message to appear");
		const cmd = msg.trim();
		aggregator.parse(cmd);
	}
	
	function onTick(vote) {
		console.log("Robot is moving: " + vote.name);
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

	client.on('message', onMessageHandler);
	client.connect();
	aggregator.run();

	console.log("Connected to Twitch channel: " + config.channel);
});
