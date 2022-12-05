.PHONY: clean

producer:   clean
	g++ -o producer producer.cc
consumer:   clean
	g++ -o consumer consumer.cc

clean:
	rm -f producer
	rm -f consumer

